make {

   import { C; Dox; Deb; }

   init {
      $name := "vypack";
      $ver := "20250223";
      $gitUrl := "https://github.com/Doi6doi/vypack";
      $author := "Várnagy Zoltán";

      $dep := "c.dep";
      $vpc := "vypack.c";
      $cs := ["tools.c",$vpc,"str.c"];
      case ( system() ) {
         "Linux": $cs += "linux.c";
         "Windows": $cs += "windows.c";
      }
      $hs := ["arch.h","str.h","tools.h"];
      $usg := "usage.inc";
      $desc := "docs/desc.dox";
      $os := changeExt( $cs, C.objExt() );
      $exe := $name+exeExt();
      $buildDir := "build";
      $linBinDir := "usr/bin";
      $purge := [$exe,$os,$usg,$buildDir];
   }

   target {

      /// Build vypack binary
      build {
         genUsage();
         genDep();
         genObjs();
         genExe();
      }

      /// purge generated files
      clean { purge( $purge ); }

      /// run tests
      test { make( "test" ); }

      /// build documentation
      docs { make("docs"); }

      /// debian package
      deb {
          build();
          makeDeb();
      }
      
   }
   
   function {

      /// create usage include
      genUsage() {
         if ( older( $usg, $vpc )) {
            Dox.set("bullet","  ");
            Dox.read( $vpc );
            Dox.set("outType","txt");
            if ( ! (u := Dox.writePart( "usage" )))
               fail("Usage part missing");
            echo("Creating: "+$usg);
            saveFile( $usg, C.literal(u) );
         }
      }

      /// scan dependencies
      genDep() {
         if ( older( $dep, $cs + $hs ))
            C.depend( $dep, $cs );
      }

      /// build object files
      genObjs() {
         ds := C.loadDep( $dep );
         foreach ( c | $cs ) {
            o := changeExt( c, C.objExt() );
            if ( older( o, ds[o] ))
               C.compile( o, c );
         }
      }

      /// build exe
      genExe() {
         if ( older( $exe, $os ))
            C.link( $exe, $os );
      }

      makeDeb() {
         mkdir( $buildDir );
         // create executable
         bbDir := path( $buildDir, $linBinDir );
         mkdir( bbDir );
         bexe := path( bbDir,$exe );
         copy( $exe, bexe );
         setPerm( bexe, "x" );
         // create description
         bdDir := path( $buildDir, "DEBIAN" );
         mkdir( bdDir );
         Dox.read($desc);
         Dox.set("outType","txt");
         ds := replace(Dox.write(), "\n", "\n " );
         // create DEBIAN/control
         cnt := [
            "Package: "+$name,
            "Version: "+$ver,
            "Architecture: "+Deb.arch( arch() ),
            "Maintainer: "+$author+" <"+$gitUrl+">",
            "Description: "+ds
         ];
         saveFile( path( bdDir, "control" ), implode("\n",cnt) );
         // build package
         Deb.build( $buildDir );
      }
   }

}
