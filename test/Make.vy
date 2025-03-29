make {

   init {
      $C := tool("C",{show:1});
   
      $tests := ["ls","cat","hwphp","hwjs","hwpy"];
      $cs := ["strs"];
      $vypack := path("..","vypack");
      $purge := changeExt( $tests+$cs, exeExt() );
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
/*         foreach ( t | $tests ) {
            e := changeExt( t, exeExt() );
            a := changeExt( t, ".args" );
            if ( older( e, [a,$vypack] )) {
               x := loadFile( a );
               exec( $vypack+" -o "+e+" "+x );
            }
         }
*/
         $C.set("incDir","..");
         foreach ( c | $cs ) {
            s := changeExt( c, ".c" );
            e := changeExt( c, exeExt() );
            if ( older(e,s))
               $C.build(e,[s,"../str.o"]);
         } 
      }

      run {
         foreach ( t | $tests )
            exec( path(".",changeExt(t,exeExt())));
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


