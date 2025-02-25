make {

   import { C; }

   init {
      $ver := "20250224";
//      $tests := ["hwphp","hwjs","hwpy"];
      $tests := ["hwphp","hwpy"];
      $cs := ["strs"];
      case (system()) {
         "Windows": $tests += ["ls","cat"];
      }
      $cs := [];
      $vypack := path("..","vypack"+exeExt() );
      $purge := ["*"+C.objExt(),changeExt( $tests+$cs, exeExt())];
      C.set("show",1);
   }

   target {

      test {
         build();
//         run();
      }

      up {
         make("..");
      }

      clean {
         purge( $purge );
      }

      build {
         foreach ( t | $tests )
            buildTest( t );
         foreach ( c | $cs )
            buildC( c );
      }
       
      run {
         foreach ( t | $tests )
            exec( path(".",changeExt(t,exeExt())));
      }
   }

   function {
   
      buildTest( t ) {
         e := changeExt( t, exeExt() );
         case (t) {
            "hwphp": a := "-x \""+which("php")+"\" -a \"%vypack%/hw.php\" -f hw.php -v "+$ver;
            "hwjs": a := "-x \""+which("node")+"\" -a \"%vypack%/hw.js\" -f hw.js -v "+$ver;
            "hwpy": a := "-x \""+which("python")+"\" -a \"%vypack%/hw.js\" -f hw.js -v "+$ver;
            else fail("Unknown test:"+t);
         }
         if ( older( e, $vypack )) {
            echo("Generating "+e );
            exec( $vypack+" -o "+e+" "+a );
         }
      }
         
/* 
          
          {
            e := changeExt( t, exeExt() );
            a := changeExt( t, ".args" );
            if ( older( e, [a,$vypack] )) {
               x := loadFile( a );
               exec( $vypack+" -o "+e+" "+x );
            }
         }
         foreach ( c | $cs ) {
         } 
*/

      buildC( c ) {
         C.set("incDir","..");
         s := changeExt( c, ".c" );
         e := changeExt( c, exeExt() );
         if ( older(e,s))
            C.build(e,[s,"../str"+C.objExt()]);
      }

   }   

}

ITEMS=ls cat hwphp hwjs hwpy
VYPACK=../vypack
CAT=cat
RM=rm -f

test: $(ITEMS:=.run)

clean:
	$(RM) *.elf

%.run: %.elf
	./$^

%.elf: %.args $(VYPACK)
	$(VYPACK) -o $@ `$(CAT) $<`


