make {

   init {
      $Dox := tool("Dox", {linkTail:".html", style:"style.css"});
   
      $ixx := "index.dox";
      $dox := "doc.dox";
      $desc := "desc.dox";
      $inx := "install.dox";

      $rm := "../README.md";
      $ixh := "index.html";
      $doh := "Documentation.html";
      $inh := "Install.html";

      $c  := "../vypack.c";
      $base := "https://doi6doi.github.io/vypack/";
      $purge := [$ixh,$rm,$doh,$inh];
   }

   target {

      /// build all documentation
      build {
         index();
         install();
         doc();
      }

      /// purge generated files
      clean { purge( $purge ); }
   }

   function {

      /// build index.html and README.md
      index() {
         links(true);
         if ( older( $rm, $ixx ))
            $Dox.build( $rm, $ixx );
         links(false);
         if ( older( $ixh, $ixx ))
            $Dox.build( $ixh, $ixx );
      }

      /// build Install.html
      install() {
         links(false);
         if ( older( $inh, $inx ))
            $Dox.build( $inh, $inx );
      }

      /// build Documentation.html
      doc() {
         links(false);
         if ( older( $doh, [$dox,$c,$desc] )) {
            echo( "Building "+$doh );
            $Dox.set({outType:"html", bullet:null});
            d := "";
            $Dox.read($dox);
            d += $Dox.writePart("head");
            $Dox.read($desc);
            d += $Dox.write();
            $Dox.read($dox);
            d += $Dox.writePart("works");
            $Dox.read($c);
            d += $Dox.writePart("usage");
            $Dox.read($dox);
            d += $Dox.writePart("examples");
            saveFile( $doh, d );
         }
      }

      /// turn full links on or off
      links(x) {
         if (x)
            $Dox.set("linkHead",$base);
            else $Dox.set("linkHead",null);
      }

   }

}
