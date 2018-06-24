

   /// --- copied from Halfcone.cxx

#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoManager.h"
#include "TGeoCompositeShape.h"
#include "TGeoShape.h"
#include "TGeoCone.h"
#include "TGeoVolume.h"
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoTube.h"
#include "TGeoTrd1.h"
#include "TMath.h"
#include "TGeoXtru.h"

//#include "/home/carlos/alice2/O2/Detectors/ITSMFT/MFT/base/include/MFTBase/HalfSupport.h"
#include "MFTBase/HalfSupport.h"

using namespace o2::MFT;

ClassImp(o2::MFT::HalfSupport);

//_____________________________________________________________________________
HalfSupport::HalfSupport():TNamed(),mHalfSupport(nullptr)
{
  
  // default constructor
  
}

//_____________________________________________________________________________
HalfSupport::~HalfSupport() = default;

//_____________________________________________________________________________
TGeoVolumeAssembly* HalfSupport::createHalfSupport(Int_t half)
{
  
      auto *HalfSupportVolume = new TGeoVolumeAssembly("HalfSupportVolume");
  
  
  
  
        // TGeoMedium *kMedAlu = gGeoManager->GetMedium("MFT_Alu$");
 ///////  add->
        ///  auto *HalfSupportVolume = new TGeoVolumeAssembly("HalfSupportVolume");

    // auto *HalfSupportVolume = new TGeoVolumeAssembly("HalfSupportVolume");
     
    //--- define some materials
      TGeoMaterial *matVacuum = new TGeoMaterial("Vacuum", 0,0,0);
      TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);
    //   //--- define some media
      TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, matVacuum);
      TGeoMedium *Al = new TGeoMedium("Root Material",2, matAl);
   /////Rotation  ////
           /////// 
    TGeoRotation   *rot1 = new TGeoRotation("rot1", 180,-180,0); //

      rot1->RegisterYourself();
      TGeoRotation   *rot2 = new TGeoRotation("rot2", 90,-90,0);

      rot2->RegisterYourself();

      TGeoRotation   *rot3 = new TGeoRotation("rot3", 0,90,0);
      rot3->RegisterYourself();

      TGeoRotation  *rot_90x = new TGeoRotation("rot_90x", 0,-90,0); // half0

      rot_90x->RegisterYourself();

      TGeoRotation   *rot_base = new TGeoRotation("rot_base", 0,0,0); // rail_r
      rot_base->RegisterYourself();
      
      TGeoTranslation *tr1 = new TGeoTranslation("tr1",-74,0,82.8);  //
     // tr1->SetName("tr1");
      tr1->RegisterYourself();      

      TGeoCombiTrans *combi1 = new TGeoCombiTrans(0, -10.3, 1.29, rot1); //y=-10.80 belt
      combi1->RegisterYourself();
      TGeoCombiTrans *combi2 = new TGeoCombiTrans(-16.8, 0, 0., rot2);
       
       combi2->RegisterYourself();
      TGeoRotation *r0 = new TGeoRotation("r0",10.,0.,0.);
      r0->RegisterYourself();

    ///// 1st piece    Cross_beam_MB0----------------------///

       auto *Cross_mb0 = new TGeoVolumeAssembly("Cross_mb0");


 TGeoMedium *kMedAlu = gGeoManager->GetMedium("MFT_Alu$");

      /// variables
       // rectangular box
      Double_t x_box_mb0 =144;  // dx= 72 mm
      Double_t y_box_mb0 =6;
      Double_t z_box_mb0 =6;

      ///// holes tub  1hole tranversal
      Double_t radin_1hole_mb0=0.;
      Double_t radout_1hole_mb0=1.75;// diameter 3.5 H9
      Double_t high_1hole_mb0=7;///

      TGeoRotation *rot_1hole_mb0 = new TGeoRotation("rot_1hole_mb0", 0,90,0);
      rot_1hole_mb0->RegisterYourself();
        /// h= hole
      TGeoCombiTrans *acombi_1h_mb0 = new TGeoCombiTrans(52.,0, 0,rot_1hole_mb0);
      acombi_1h_mb0->SetName("acombi_1h_mb0");
      acombi_1h_mb0->RegisterYourself();
      TGeoCombiTrans *bcombi_1h_mb0 = new TGeoCombiTrans(-52.,0, 0,rot_1hole_mb0);//y=
      bcombi_1h_mb0->SetName("bcombi_1h_mb0");
      bcombi_1h_mb0->RegisterYourself();

      ////////// 2hole coaxial
      Double_t radin_2hole_mb0=0.;
      Double_t radout_2hole_mb0=1.5;// diameter M3
      Double_t high_2hole_mb0=12;///

      TGeoRotation *rot_2hole_mb0 = new TGeoRotation("rot_2hole_mb0", 90,90,0);
      rot_2hole_mb0->SetName("rot_2hole_mb0");
      rot_2hole_mb0->RegisterYourself();

      TGeoCombiTrans *combi_2hole_mb0 = new TGeoCombiTrans(67.,0, 0,rot_2hole_mb0);
      combi_2hole_mb0->SetName("combi_2hole_mb0");
      combi_2hole_mb0->RegisterYourself();

      TGeoCombiTrans *combi_2hole_mb0_b = new TGeoCombiTrans(-67,0, 0,rot_2hole_mb0);//y=
      combi_2hole_mb0_b->SetName("combi_2hole_mb0_b");
      combi_2hole_mb0_b->RegisterYourself();

    // shape for cross_mb0 

      auto *s_box_mb0 =new TGeoBBox("S_BOX_MB0", x_box_mb0/2,y_box_mb0/2,z_box_mb0/2);
      auto *s_1hole_mb0=new TGeoTube("S_1HOLE_MB0",radin_1hole_mb0,radout_1hole_mb0,high_1hole_mb0/2);
      auto *s_2hole_mb0=new TGeoTube("S_2HOLE_MB0",radin_2hole_mb0,radout_2hole_mb0,high_2hole_mb0/2);

   ///composite shape for mb0
   
      auto * c_mb0_Shape_0 = new TGeoCompositeShape("c_mb0_Shape_0","S_BOX_MB0 - S_1HOLE_MB0:acombi_1h_mb0 - S_1HOLE_MB0:bcombi_1h_mb0  - S_2HOLE_MB0:combi_2hole_mb0 -S_2HOLE_MB0:combi_2hole_mb0_b");

       ///////////////////
      auto * cross_mb0_Volume = new TGeoVolume("cross_mb0_Volume",c_mb0_Shape_0,kMedAlu);

     Cross_mb0->AddNode(cross_mb0_Volume,1); 

////////////////////////////////////////////

    ///// 2nd piece   ------ cross beam--MFT---------(cbeam)-----/// ========

    auto *Cross_mft = new TGeoVolumeAssembly("Cross_mft");
      /// variables
    
      // using the same "box" of the 1 piece
    ////////// 2hole coaxial
      Double_t radin_hole_cbeam=0.;
      Double_t radout_hole_cbeam=1.5;// diameter M3
      Double_t high_hole_cbeam=9.1;///

     // same rotation in tub aximatAl "rot_2hole_mb0"

      TGeoCombiTrans *combi_hole_1cbeam = new TGeoCombiTrans(68.,0, 0,rot_2hole_mb0);
      combi_hole_1cbeam->SetName("combi_hole_1cbeam");
      combi_hole_1cbeam->RegisterYourself();

      TGeoCombiTrans *combi_hole_2cbeam = new TGeoCombiTrans(-68.,0, 0,rot_2hole_mb0);
      combi_hole_2cbeam->SetName("combi_hole_2cbeam");
      combi_hole_2cbeam->RegisterYourself();
       
     // shape for shape cross beam  

      auto *s_hole_cbeam=new TGeoTube("S_HOLE_CBEAM",radin_hole_cbeam,radout_hole_cbeam,high_hole_cbeam/2);
   
      /// composite shape for cross beam  (using the same box of mb0)
      auto * c_cbeam_Shape = new TGeoCompositeShape("c_cbeam_Shape","S_BOX_MB0 - S_HOLE_CBEAM:combi_hole_1cbeam - S_HOLE_CBEAM:combi_hole_2cbeam");

      ///////////////////
      auto * Cross_mft_Volume = new TGeoVolume("Cross_mft_Volume",c_cbeam_Shape,kMedAlu);

    Cross_mft->AddNode(Cross_mft_Volume,1); 
////////////////////////////////////////////

   
    ////3th piece    Framework front
    auto *Fra_front = new TGeoVolumeAssembly("Fra_front");
     
      Double_t x_box_up =6;  // mm
      Double_t y_box_up =6.05;
      Double_t z_box_up =28.4;
        //hole up //
      Double_t dia_tub_up=3.5;
      Double_t high_tub_up=6.5;
      //hole down////
      Double_t dia_tubdown =3.5;
      Double_t high_tubdown =6.8; 
      // 
      Double_t x_boxA_down=8;
      Double_t y_boxA_down=6;
      Double_t z_boxA_down=6;
      //
      Double_t x_boxB_down=6;
      Double_t y_boxB_down=6.05;
      Double_t z_boxB_down=12.6; // 12.6
      //seg tub
      Double_t radin_segtub=169;
      Double_t radout_segtub=175;
      Double_t high_segtub=6.;
      Double_t ang_in_segtub=212.1;
      Double_t ang_fin_segtub=241.92; //  

      /////// trans. rot.
      TGeoCombiTrans *combi_3a = new TGeoCombiTrans(-74, 0, 89.75,rot2);//y=
      combi_3a->SetName("combi_3a");
      combi_3a->RegisterYourself();
     
      TGeoTranslation *tr1_tub1 = new TGeoTranslation("tr1_tub1",0,0.,30.75);
      tr1_tub1->RegisterYourself();

      TGeoCombiTrans *combi_3b = new TGeoCombiTrans(71.18, 0, 161.6,rot3);//x=7.35, y=0, z=15.79
      combi_3b->SetName("combi_3b");
      combi_3b->RegisterYourself();
   
      TGeoTranslation *tr_2_box = new TGeoTranslation("tr_2_box",-4,0,7);  //
      tr_2_box->RegisterYourself();

      TGeoTranslation *tr3_box = new TGeoTranslation("tr3_box",-11,0,6.3);  //
      tr3_box->RegisterYourself();

      TGeoTranslation *tr_tubdown = new TGeoTranslation("tr_tubdown",-4,0,7);  //
      tr_tubdown->RegisterYourself();

      /////// shape for framewor front
      TGeoShape *help = new TGeoBBox("help", 6,3,13.3);//

      TGeoShape *s_box_up = new TGeoBBox("BOX_UP", x_box_up/2,y_box_up/2,z_box_up/2);

      TGeoShape *s_tub_up=new TGeoTube("TUB_UP",0.,dia_tub_up/2,high_tub_up/2); //
      TGeoShape *s_seg_tub=new TGeoTubeSeg("SEG_TUB",radin_segtub,radout_segtub,high_segtub/2,ang_in_segtub,ang_fin_segtub);//r_in,r_out,dZ,ang,ang

      TGeoShape *s_boxB_down = new TGeoBBox("BOXB_DOWN", x_boxB_down/2,y_boxB_down/2,z_boxB_down/2);
        
      TGeoShape * s_boxA_down = new TGeoBBox("BOXA_DOWN", x_boxA_down/2,y_boxA_down/2,z_boxA_down/2);
    
      TGeoShape *s_tubdown=new TGeoTube("S_TUBDOWN",0.,dia_tubdown/2,high_tubdown/2); 
      TGeoShape *Fra_front_Upper = new TGeoBBox("Fra_front_Upper", 1, 5, 1.7);
      TGeoShape *Half_1_Upper = new TGeoBBox("Half_1_Upper", 2, 2, 1.3);

     //Composite shapes for Fra_front
      auto * Fra_front_Shape_0 = new TGeoCompositeShape("Fra_front_Shape_0","BOX_UP:tr1 +SEG_TUB:combi_3b+ BOXB_DOWN:tr3_box + BOXA_DOWN:tr_2_box"); 

      auto * Fra_front_Shape_1 = new TGeoCompositeShape("Fra_front_Shape_1","Fra_front_Shape_0 - S_TUBDOWN:tr_tubdown -TUB_UP:combi_3a");
      auto * Fra_front_Shape_2 = new TGeoCompositeShape("Fra_front_Shape_2","SEG_TUB+BOXB_DOWN ");


      auto * Fra_front_Volume = new TGeoVolume("Fra_front_Volume",Fra_front_Shape_1,Al);
  
   Fra_front->AddNode(Fra_front_Volume,1,rot_90x);  //rot_90x
       
 ///////////////////////////////////////////////-----

     //4th piece "BASE" framework half support /////////////
   auto *base = new TGeoVolumeAssembly("base");

   //seg tub  disc
      Double_t radin_disc=236;
      Double_t radout_disc=303;
      Double_t high_disc=13.5;
      Double_t ang_in_disc=180;
      Double_t ang_fin_disc=360; 

    ///// holes tub  1hole tranversal o3.5
      Double_t radin_holeB=0.;
      Double_t radout_holeB=1.75;// diameter 3.5 H11
      Double_t high_holeB=15.;///
      TGeoTranslation *tr1_holeB = new TGeoTranslation("tr1_holeB",-75,-288,0);
      tr1_holeB->RegisterYourself();

      TGeoTranslation *tr2_holeB = new TGeoTranslation("tr2_holeB",75,-288,0);
      tr2_holeB->RegisterYourself();

     //box 1
      Double_t x_1box=610;
      Double_t y_1box=130;
      Double_t z_1box=14;
      //box 2
      Double_t x_2box=512;
      Double_t y_2box=146;
      Double_t z_2box=14;
      //box 3
      Double_t x_3box=451;
      Double_t y_3box=238.12;
      Double_t z_3box=14;
     //seg tub hole 
      Double_t radin_1hole=293;
      Double_t radout_1hole=303;
      Double_t high_1hole=14;
      Double_t ang_in_1hole=205;
      Double_t ang_fin_1hole=225; 
      //seg tub 2 hole 
      Double_t radin_2hole=230;
      Double_t radout_2hole=255;
      Double_t high_2hole=14;
      Double_t ang_in_2hole=207.83;
      Double_t ang_fin_2hole=249.998; 
      //seg tub 3 ARC central xy  SEG_3ARC U
      Double_t radin_3hole=255;
      Double_t radout_3hole=275;//304
      Double_t high_3hole=13.5;
      Double_t ang_in_3hole=255.253;
      Double_t ang_fin_3hole=284.746; //284.746
    //// hole central down |_|   since x=-70 to 0
      Double_t xc_box= 70 ;
      Double_t yc_box= 57.72;
      Double_t zc_box= 14;
     
      TGeoTranslation *tr_cbox = new TGeoTranslation("tr_cbox",-xc_box/2,-radout_disc+8.88,0);  //
      tr_cbox->RegisterYourself(); 
     //box 4 lamine 1
      Double_t x_labox=600;
      Double_t y_labox=303;
      Double_t z_labox=3.05;
      TGeoTranslation *tr_la = new TGeoTranslation("tr_la",0,-y_labox/2-93,high_disc/2);  //
      tr_la->RegisterYourself();

      /////box 5   lamin 2
      Double_t x_2labox=512;
      Double_t y_2labox=28.; //C-B
      Double_t z_2labox=3.03;
      TGeoTranslation *tr_2la = new TGeoTranslation("tr_2la",0,-81,high_disc/2);  //
      tr_2la->RegisterYourself();

      ///////// circular border C //--SEG_BORD   ----
      //seg tub 3 xy 
      Double_t radin_bord=5;
      Double_t radout_bord=9;//304
      Double_t high_bord=13.55;///13.5
      Double_t ang_in_bord=0;
      Double_t ang_fin_bord=90; 
      /////TGeoRotation *rot_bord1 = new TGeoRotation("rot_bord1", ang_in_1hole +0.0167,0,0);
      TGeoRotation *rot1_bord1 = new TGeoRotation("rot1_bord1", 14.8,0,0);
      rot1_bord1->RegisterYourself();
      TGeoCombiTrans *combi_bord1 = new TGeoCombiTrans(-267.995,-130.215, 0,rot1_bord1);//y=
      combi_bord1->SetName("combi_bord1");
      combi_bord1->RegisterYourself();

      TGeoRotation *rot2_bord1 = new TGeoRotation("rot2_bord1", -50,0,0);
      rot2_bord1->RegisterYourself();
      TGeoCombiTrans *combi2_bord1 = new TGeoCombiTrans(-213.795,-207.636, 0,rot2_bord1);//y=
      combi2_bord1->SetName("combi2_bord1");
      combi2_bord1->RegisterYourself();
  //////// /__0/
      TGeoRotation *rot1_bord2 = new TGeoRotation("rot1_bord2", 250,0,0);
      rot1_bord2->RegisterYourself();
      TGeoCombiTrans *combi1_bord2 = new TGeoCombiTrans(-90.527,-233.006, 0,rot1_bord2);//y=
      combi1_bord2->SetName("combi1_bord2");
      combi1_bord2->RegisterYourself();
     /////// |°____°|
      TGeoRotation *rot_cent_bord = new TGeoRotation("rot_cent_bord", 90,0,0);
      rot_cent_bord->RegisterYourself();
      TGeoCombiTrans *combi_cent_bord = new TGeoCombiTrans(-65,-270.94, 0,rot_cent_bord);//y=
      combi_cent_bord->SetName("combi_cent_bord");
      combi_cent_bord->RegisterYourself();
   ///////////////// box tonge
      Double_t x_tong=20;
      Double_t y_tong=28.1;
      Double_t z_tong=13.5;
      TGeoTranslation *tr_tong= new TGeoTranslation("tr_tong",0,-286,0);  //
      tr_tong->RegisterYourself();
     //// circular central hole1 to conexion with other parts 
      Double_t radin_hole1=0;
      Double_t radout_hole1=4;//304
      Double_t high_hole1=13.6;///13.5
      TGeoTranslation *tr_hole1= new TGeoTranslation("tr_hole1",0,-280,0);  //tonge
      tr_hole1->RegisterYourself();
       
      TGeoTranslation *tr2_hole1= new TGeoTranslation("tr2_hole1",-265,-85,0); //left
      tr2_hole1->RegisterYourself();
       
      TGeoTranslation *tr3_hole1= new TGeoTranslation("tr3_hole1",265,-85,0);  //right
      tr3_hole1->RegisterYourself();
  /////////////////////////////////////////
       /// circular hole2 ; hole2 r=6.7
      Double_t radin_hole2=0;
      Double_t radout_hole2=3.35;// diameter 6.7
      Double_t high_hole2=13.6;///13.5
      TGeoTranslation *tr1_hole2= new TGeoTranslation("tr1_hole2",-280,-85,0);  //
      tr1_hole2->RegisterYourself();
       
      TGeoTranslation *tr2_hole2= new TGeoTranslation("tr2_hole2",280,-85,0);  //
      tr2_hole2->RegisterYourself();


//////////// hole "0" two tubs together  
      Double_t radin_T1=3.25; // diam 6.5
      Double_t radout_T1=5.5;// dia 11   
      Double_t high_T1=12;///  dz 6
       
      Double_t radin_T2=0;
      Double_t radout_T2=11;//
      Double_t high_T2=12;///  dz 6
      ///...................................... .....
       
      ///////// shape for base --

      auto *s_disc=new TGeoTubeSeg("S_DISC",radin_disc,radout_disc,high_disc/2,ang_in_disc,ang_fin_disc);

      auto *s_1box =new TGeoBBox("BOX1", x_1box/2,y_1box/2,z_1box/2);
      auto *s_2box =new TGeoBBox("BOX2", x_2box/2,y_2box/2,z_2box/2);
      auto *s_3box =new TGeoBBox("BOX3", x_3box/2,y_3box/2,z_3box/2);
      auto *s_labox =new TGeoBBox("LA_BOX", x_labox/2,y_labox/2,z_labox/2);
      auto *s_2labox =new TGeoBBox("LA_2BOX", x_2labox/2,y_2labox/2,z_2labox/2);
      auto *s_cbox =new TGeoBBox("CENTRAL_BOX", xc_box/2,yc_box/2,zc_box/2);
      auto *s_tongbox =new TGeoBBox("TONG_BOX", x_tong/2,y_tong/2,z_tong/2);
   
      TGeoShape *s_seg_1hole=new TGeoTubeSeg("SEG_1HOLE",radin_1hole,radout_1hole,high_1hole/2,ang_in_1hole,ang_fin_1hole);//r_in,r_out,dZ,ang,ang 
      TGeoShape *s_seg_2hole=new TGeoTubeSeg("SEG_2HOLE",radin_2hole,radout_2hole,high_2hole/2,ang_in_2hole,ang_fin_2hole);
      TGeoShape *s_seg_3hole=new TGeoTubeSeg("SEG_3ARC",radin_3hole,radout_3hole,high_3hole/2,ang_in_3hole,ang_fin_3hole);// |u|
      TGeoShape *s_seg_bord=new TGeoTubeSeg("SEG_BORD",radin_bord,radout_bord,high_bord/2,ang_in_bord,ang_fin_bord);

      auto *s_circ_hole=new TGeoTube("S_CIRC_HOLE1",radin_hole1,radout_hole1,high_hole1/2);
   
      auto *s_circ_hole2=new TGeoTube("S_CIRC_HOLE2",radin_hole2,radout_hole2,high_hole2/2);

      auto *s_circ_holeB=new TGeoTube("S_CIRC_HOLEB",radin_holeB,radout_holeB,high_holeB/2);
 
     //// composite shape for base ----

      auto * base_Shape_0 = new TGeoCompositeShape("base_Shape_0","S_DISC -BOX1 -BOX2 -BOX3 - S_CIRC_HOLEB:tr1_holeB- S_CIRC_HOLEB:tr2_holeB ");
      auto * base_Shape_1 = new TGeoCompositeShape("base_Shape_1","(SEG_1HOLE -SEG_BORD:combi_bord1 -SEG_BORD:combi2_bord1) + SEG_2HOLE-SEG_BORD:combi1_bord2 + CENTRAL_BOX:tr_cbox ");

      auto * base_Shape_2 = new TGeoCompositeShape("base_Shape_2"," SEG_3ARC +SEG_BORD:combi_cent_bord   ");  //-SEG_BORD:combi_cent_bord

      auto * base_Shape_3 = new TGeoCompositeShape("base_Shape_3"," LA_BOX:tr_la+ LA_2BOX:tr_2la  ");

      auto * base_Shape_4 = new TGeoCompositeShape("base_Shape_4","base_Shape_0 -base_Shape_1 - base_Shape_1:rot1 + base_Shape_2  +TONG_BOX:tr_tong -S_CIRC_HOLE1:tr_hole1 -S_CIRC_HOLE1:tr2_hole1 -S_CIRC_HOLE1:tr3_hole1 -S_CIRC_HOLE2:tr1_hole2-S_CIRC_HOLE2:tr2_hole2 -base_Shape_3 ");  


     auto * base_Shape_5 = new TGeoCompositeShape("base_Shape_5","S_DISC-BOX1 -BOX2 -BOX3 -SEG_1HOLE -SEG_2HOLE +SEG_3ARC -SEG_1HOLE:rot1-SEG_2HOLE:rot1 - CENTRAL_BOX:tr_cbox - LA_BOX:tr_la - LA_2BOX:tr_2la  + SEG_BORD  "); 
 
     auto * base0_Volume = new TGeoVolume("base0_Volume",base_Shape_0,kMedAlu);
     auto * base1_Volume = new TGeoVolume("base1_Volume",base_Shape_1,kMedAlu);
     auto * base2_Volume = new TGeoVolume("base2_Volume",base_Shape_2,kMedAlu);
     auto * base3_Volume = new TGeoVolume("base3_Volume",base_Shape_3,kMedAlu);
     auto * base4_Volume = new TGeoVolume("base4_Volume",base_Shape_4,kMedAlu);


   base->AddNode(base4_Volume,2,rot_base); 
 //////////////////////////////////////////


   ///5th piece MIDLE ------ Framework midle---

     auto *midle = new TGeoVolumeAssembly("Midle");
  
       ////box up to quit and to join
       Double_t x_midle=8; //dx=4
       Double_t y_midle=34.95; //y=34.9
       Double_t z_midle=6.2;   //z=6
      // tr1 to join with arc
       TGeoTranslation *tr1_midle_box= new TGeoTranslation("tr1_midle_box",-144,-7.45,0);  //-152,-17.45,0
       tr1_midle_box->RegisterYourself();
       //tr2 to quiet
       TGeoTranslation *tr2_midle_box= new TGeoTranslation("tr2_midle_box",-152,-7.45,0);  //-152,-17.45,0
       tr2_midle_box->RegisterYourself();

    ///////////////////////////
      ////box down_1
       Double_t x_midle_d1box=4; //dx=4
       Double_t y_midle_d1box=2.8;
       Double_t z_midle_d1box=6.6;
       TGeoTranslation *tr_midle_d1box= new TGeoTranslation("tr_midle_d1box",-73,-119.6,0);  //81
       tr_midle_d1box->RegisterYourself();

     ///////////////////////////
      ////box down_2
      Double_t x_midle_d2box=8; //dx=4
      Double_t y_midle_d2box=10;
      Double_t z_midle_d2box=6.6; // 6.5 -> 6.6 to quit
      TGeoTranslation *tr_midle_d2box= new TGeoTranslation("tr_midle_d2box",-75,-126.249,0);  //81
      tr_midle_d2box->RegisterYourself();
  
      ///////////////////
       // circ part 
      Double_t radin_midle=140;
      Double_t radout_midle=150;//
      Double_t high_midle=6;///
      Double_t ang_in_midle=180;
      Double_t ang_fin_midle=238.21;  //alfa=57.60   .(first 237.31)

      /////////////////////////////////////////
       /// circular hole1 ; hole_midle d=3.5
      Double_t radin_mid_1hole=0.;
      Double_t radout_mid_1hole=1.75;// diameter 3.5
      Double_t high_mid_1hole=24;///

      TGeoRotation *rot_mid_1hole = new TGeoRotation("rot_mid_1hole", 90,90,0);
      rot_mid_1hole->RegisterYourself();
      TGeoCombiTrans *combi_mid_1tubhole = new TGeoCombiTrans(-142,3.25, 0,rot_mid_1hole);//
      combi_mid_1tubhole->SetName("combi_mid_1tubhole");
      combi_mid_1tubhole->RegisterYourself();

      /////////////////////////////////////////
      /// circular hole2 ; hole_midle d=3
      Double_t radin_mid_2hole=0;
      Double_t radout_mid_2hole=1.5;// diameter 3
      Double_t high_mid_2hole=18.5;///
    
      TGeoCombiTrans *combi_mid_2tubhole = new TGeoCombiTrans(-77,-123.55, 0,rot_mid_1hole);//x=81
      combi_mid_2tubhole->SetName("combi_mid_2tubhole");
      combi_mid_2tubhole->RegisterYourself();

      //////////////////////////////////////////////////////   

     
     ///////// shape for midle 

     auto *s_midle_box =new TGeoBBox("S_MIDLE_BOX", x_midle/2,y_midle/2,z_midle/2);

     auto *s_midle_d1box =new TGeoBBox("S_MIDLE_D1BOX", x_midle_d1box/2,y_midle_d1box/2,z_midle_d1box/2);

     auto *s_midle_d2box =new TGeoBBox("S_MIDLE_D2BOX", x_midle_d2box/2,y_midle_d2box/2,z_midle_d2box/2);   

     auto *s_arc_midle=new TGeoTubeSeg("S_ARC_MIDLE",radin_midle,radout_midle,high_midle/2,ang_in_midle,ang_fin_midle);
  
     auto *s_mid_1tubhole=new TGeoTube("S_MID_1TUBHOLE",radin_mid_1hole,radout_mid_1hole,high_mid_1hole/2);

     auto *s_mid_2tubhole=new TGeoTube("S_MID_2TUBHOLE",radin_mid_2hole,radout_mid_2hole,high_mid_2hole/2);

     //////////////////////////////////////////composite shape for midle

     auto * midle_Shape_0 = new TGeoCompositeShape("midle_Shape_0"," S_ARC_MIDLE  + S_MIDLE_BOX:tr1_midle_box-S_MIDLE_BOX:tr2_midle_box-S_MIDLE_D1BOX:tr_midle_d1box-S_MIDLE_D2BOX:tr_midle_d2box  ");

     auto * midle_Shape_1 = new TGeoCompositeShape("midle_Shape_1"," midle_Shape_0 -S_MID_1TUBHOLE:combi_mid_1tubhole-S_MID_2TUBHOLE:combi_mid_2tubhole");

   
     auto * midle0_Volume = new TGeoVolume("midle0_Volume",midle_Shape_1,kMedAlu);

   midle->AddNode(midle0_Volume,1); 




      ////////////////////////////////////new piece ... _/   \_  
     /// Support_rail_L & Support_rail_R

   auto *rail_L_R = new TGeoVolumeAssembly("rail_L_R");

      //6 piece RAIL LEFT --RL---0000------
     auto *rail_L = new TGeoVolumeAssembly("rail_L");

    /////////////////VARIAb
            ////box down_2
     Double_t x_RL_1box=30; //dx=15
     Double_t y_RL_1box=12.1;  // dy=6, -dy=6
     Double_t z_RL_1box=8; // dz=4     to quit
     TGeoTranslation *tr_RL_1box= new TGeoTranslation(0,y_RL_1box/2,18.25);  //81
     tr_RL_1box->SetName("tr_RL_1box");
     tr_RL_1box->RegisterYourself();
     //////////////////////////////7

     TGeoXtru *xtru_RL1 = new TGeoXtru(2);
     xtru_RL1->SetName("S_XTRU_RL1");
             
     Double_t x_RL1[5]={-15,15,5,5,-15 };//93,93,73,73,-15};//vertices
     Double_t y_RL1[5]={12,12,22,82,82};//,357.5,357.5,250.78,145.91};
     xtru_RL1->DefinePolygon(5,x_RL1,y_RL1);
     xtru_RL1->DefineSection(0,-22.25,0,0,1);  //(plane,-zplane/ +zplane, x0, y0,(x/y))
     xtru_RL1->DefineSection(1,22.25,0,0,1);
    ////////////// \//

     TGeoXtru *xtru_RL2 = new TGeoXtru(2);
     xtru_RL2->SetName("S_XTRU_RL2");
     //TGeoVolume *vol_RL2 = gGeoManager->MakeXtru("S_XTRU_RL2",Al,2);
     //-TGeoXtru *xtru_RL2 = (TGeoXtru*)vol_RL2->GetShape();
    
     Double_t x_RL2[8]={-15,5,5,93,93,73,73,-15};   //vertices
     Double_t y_RL2[8]={82,82,138.63,243.5,357.5,357.5,250.78,145.91};

     xtru_RL2->DefinePolygon(8,x_RL2,y_RL2);
     xtru_RL2->DefineSection(0,7.76,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru_RL2->DefineSection(1,22.25 ,0,0,1);

/////////////////////////  ////////////////////
     ////box knee
     Double_t x_RL_kneebox=15; //dx=7.5
     Double_t y_RL_kneebox=35;  // dy=17.5
     Double_t z_RL_kneebox=15; // dz=7.5     to quit
     TGeoTranslation *tr_RL_kneebox= new TGeoTranslation(0,0,0);  //81 x =-2.5, y=145.91
     tr_RL_kneebox->SetName("tr_RL_kneebox");
     tr_RL_kneebox->RegisterYourself();

     TGeoRotation *rot_knee = new TGeoRotation("rot_knee", -40,0,0);
     rot_knee->SetName("rot_knee");
     rot_knee->RegisterYourself();
     TGeoCombiTrans *combi_knee = new TGeoCombiTrans(9.6,17.5+8.1864, 0,rot_knee);//y=
     combi_knee->SetName("combi_knee");
     combi_knee->RegisterYourself();
   /////////////////////////////////  quit diagona-> qdi/////////////////
     Double_t x_qdi_box=31; //
     Double_t y_qdi_box=71.59;  // 
     Double_t z_qdi_box=30.05; // 

     TGeoRotation *rot_qdi = new TGeoRotation("rot_qdi", 0,24.775,0);
     rot_qdi->RegisterYourself();
     TGeoCombiTrans *combi_qdi = new TGeoCombiTrans(0,55.79, -20.87,rot_qdi);//y=
     combi_qdi->SetName("combi_qdi");
     combi_qdi->RegisterYourself();
 //////////////  knee small

     TGeoXtru *xtru3_RL = new TGeoXtru(2);
     xtru3_RL->SetName("S_XTRU3_RL");
     //-TGeoVolume *vol3_RL = gGeoManager->MakeXtru("S_XTRU3_RL",Al,2);
     //-TGeoXtru *xtru3_RL = (TGeoXtru*)vol3_RL->GetShape();

     Double_t x_3RL[6]={-7.5,7.5,7.5,26.487,14.997,-7.5};   //vertices
     Double_t y_3RL[6]={-17.5,-17.5,12.03,34.65,44.311,17.5};

     xtru3_RL->DefinePolygon(6,x_3RL,y_3RL);
     xtru3_RL->DefineSection(0,-7.5,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru3_RL->DefineSection(1,7.6,0,0,1);

     TGeoTranslation *tr_vol3_RL= new TGeoTranslation(-2.5,126.6,0);  //
     tr_vol3_RL->SetName("tr_vol3_RL");
     tr_vol3_RL->RegisterYourself();
/////////////////////////////////////////////
     //// circular holes  could be for rail R and L ..
     /// circular hole1_RL (a(6,22)); hole_midle d=6.5 H11
     Double_t radin_RL1hole=0.;
     Double_t radout_RL1hole=3.25;// diameter 3.5
     Double_t high_RL1hole=10;///

     TGeoRotation *rot_RL1hole = new TGeoRotation("rot_RL1hole", 0,0,0);
     rot_RL1hole->RegisterYourself();
     TGeoCombiTrans *combi_RL1hole = new TGeoCombiTrans(7,6, 18.5,rot_RL1hole);//y=
     combi_RL1hole->SetName("combi_RL1hole");
     combi_RL1hole->RegisterYourself();
//////////////////////////////// similar hole for R -- Join.
     /// circular hole_ir. diameter=M3 (3 mm)) prof trou:8, tar:6mm 
     Double_t radin_ir_railL=0.;
     Double_t radout_ir_railL=1.5;// diameter 3
     Double_t high_ir_railL=39;// 19
     TGeoRotation *rot_ir_RL = new TGeoRotation("rot_ir_RL", 90,90,0);
     rot_ir_RL->RegisterYourself();
        //// in y = l_253.5 - 6. enter in (0,6,0)
     TGeoCombiTrans *combi_ir1_RL = new TGeoCombiTrans(86.2,247.5, 15,rot_ir_RL);
     combi_ir1_RL->SetName("combi_ir1_RL");
     combi_ir1_RL->RegisterYourself();

     TGeoCombiTrans *combi_ir2_RL = new TGeoCombiTrans(86.,331.5, 15,rot_ir_RL);
     combi_ir2_RL->SetName("combi_ir2_RL");
     combi_ir2_RL->RegisterYourself();

///////////////////////  shape for Rail L geom/////////
     auto *s_RL_1box =new TGeoBBox("S_RL_1BOX", x_RL_1box/2,y_RL_1box/2,z_RL_1box/2);
     auto *s_RL_kneebox =new TGeoBBox("S_RL_KNEEBOX", x_RL_kneebox/2,y_RL_kneebox/2,z_RL_kneebox/2);
     auto *s_qdi_box =new TGeoBBox("S_QDI_BOX", x_qdi_box/2,y_qdi_box/2,z_qdi_box/2);
     auto *s_RL1hole=new TGeoTube("S_RL1HOLE",radin_RL1hole,radout_RL1hole,high_RL1hole/2);

     auto *s_irL_hole=new TGeoTube("S_irL_HOLE",radin_ir_railL,radout_ir_railL,high_ir_railL/2);

     
////////////////////// composite shape for rail L //////////////////////
 
     auto * RL_Shape_0 = new TGeoCompositeShape("RL_Shape_0"," S_XTRU3_RL:tr_vol3_RL +S_XTRU_RL1 + S_XTRU_RL2 +S_RL_1BOX:tr_RL_1box -S_QDI_BOX:combi_qdi");  //+S_RL_KNEEBOX:combi_knee + +

      
   //////////////////////////////////////-------////////

     TGeoVolume *rail_L_vol0 = new TGeoVolume("RAIL_L_VOL0",RL_Shape_0,kMedAlu);

   rail_L->AddNode(rail_L_vol0,1,new TGeoTranslation(0., 0., 15.) );
 
    
     ////__7___ pice RAIL RIGHT  //////////////////////////////////-----

    auto *rail_R = new TGeoVolumeAssembly("rail_R");

    /////////////////VARIAb
      /// variable
        ////box 
     Double_t x_RR_1box=30; //dx=15
     Double_t y_RR_1box=12;  // dy=6, -dy=6
     Double_t z_RR_1box=8; // dz=4     to quit
     TGeoTranslation *tr_RR_1box= new TGeoTranslation("tr_RR_1box",0,6,18.25);  //81
     tr_RR_1box->RegisterYourself();
     ////////////////////////////////7

     TGeoXtru *xtru_RR1 = new TGeoXtru(2);
     xtru_RR1->SetName("S_XTRU_RR1");
     //-TGeoVolume *vol_RR1 = gGeoManager->MakeXtru("S_XTRU_RR1",Al,2);
     //-TGeoXtru *xtru_RR1 = (TGeoXtru*)vol_RR1->GetShape();
          
     Double_t x_RR1[5]={-15,-5,-5,15,15 };//C,D,K,L,C' //vertices
     Double_t y_RR1[5]={12,22,82,82,12};//,357.5,357.5,250.78,145.91};

     xtru_RR1->DefinePolygon(5,x_RR1,y_RR1);
     xtru_RR1->DefineSection(0,-22.25,0,0,1);  //(plane,-zplane/ +zplane, x0, y0,(x/y))
     xtru_RR1->DefineSection(1,22.25,0,0,1);

    ////////////// \//////////

     TGeoXtru *xtru_RR2 = new TGeoXtru(2);
     xtru_RR2->SetName("S_XTRU_RR2");
     //-TGeoVolume *vol_RR2 = gGeoManager->MakeXtru("S_XTRU_RR2",Al,2);
     //-TGeoXtru *xtru_RR2 = (TGeoXtru*)vol_RR2->GetShape();

     Double_t x_RR2[8]={-5,-5,-93,-93,-73,-73,15,15};//K,E,F,G,H,I,J,L//vertices
     Double_t y_RR2[8]={82,138.63,243.5,357.5,357.5,250.78,145.91,82};

     xtru_RR2->DefinePolygon(8,x_RR2,y_RR2);
     xtru_RR2->DefineSection(0,7.76,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru_RR2->DefineSection(1,22.25 ,0,0,1);
/////////////////////////  ////////////////////
     //////////////  knee smmatAll

     TGeoXtru *xtru3_RR = new TGeoXtru(2);
     xtru_RL1->SetName("S_XTRU3_RR");
     //-TGeoVolume *vol3_RR = gGeoManager->MakeXtru("S_XTRU3_RR",Al,2);
     //-TGeoXtru *xtru3_RR = (TGeoXtru*)vol3_RR->GetShape();

     Double_t x_3RR[6]={10,10,-12.497,-22.138,-5,-5};//R,Q,P,O,N.M   //vertices
     Double_t y_3RR[6]={109.1,144.1,170.911,159.421,138.6,109.1};

     xtru3_RR->DefinePolygon(6,x_3RR,y_3RR);
     xtru3_RR->DefineSection(0,-7.5,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru3_RR->DefineSection(1,7.8,0,0,1);

     TGeoTranslation *tr_vol3_RR= new TGeoTranslation("tr_vol3_RR",-2.5,126.6,0);  //
     tr_vol3_RR->RegisterYourself();

      /////////////////////////////////  quit diagona-> qdi/////////////////
     Double_t x_qdi_Rbox=31; //dx=15
     Double_t y_qdi_Rbox=71.59;  // dy=17.5
     Double_t z_qdi_Rbox=30.05; // dz=7.5     to quit

     TGeoRotation *rot_Rqdi = new TGeoRotation("rot_Rqdi", 0,24.775,0);
     rot_Rqdi->RegisterYourself();
     TGeoCombiTrans *combi_Rqdi = new TGeoCombiTrans(0,55.79, -20.87,rot_Rqdi);//y=
     combi_Rqdi->SetName("combi_Rqdi");
     combi_Rqdi->RegisterYourself();

    ////////////////////___holes__/////

    /// circular hole_a. diameter=6.5 (a(6,22)); hole_midle d=6.5 H11
     Double_t radin_a_rail=0.;
     Double_t radout_a_rail=3.25;// diameter 3.5
     Double_t high_a_rail=8.2;///

     TGeoTranslation *tr_a_RR= new TGeoTranslation("tr_a_RR",-7.,6,18.25);  //right
     tr_a_RR->RegisterYourself();
  //// circular hole_ir. diameter=M3 (3 mm)) prof trou:8, tar:6mm 
     Double_t radin_ir_rail=0.;
     Double_t radout_ir_rail=1.5;// diameter 3
     Double_t high_ir_rail=32;// 19  ---
     TGeoRotation *rot_ir_RR = new TGeoRotation("rot_ir_RR", 90,90,0);
     rot_ir_RR->RegisterYourself();
        //// in y = l_253.5 - 6. center in (0,6,0)
     TGeoCombiTrans *combi_ir_RR = new TGeoCombiTrans(-86.2,247.5, 15,rot_ir_RR);
     combi_ir_RR->SetName("combi_ir_RR");
     combi_ir_RR->RegisterYourself();

     TGeoCombiTrans *combi_ir2_RR = new TGeoCombiTrans(-86.,331.5, 15,rot_ir_RR);
     combi_ir2_RR->SetName("combi_ir2_RR");
     combi_ir2_RR->RegisterYourself();
             
     TGeoCombiTrans *combi_rail_R = new TGeoCombiTrans(241,-18.25, 0,rot_90x);//y=
     combi_rail_R->SetName("combi_rail_R");
     combi_rail_R->RegisterYourself();
     TGeoCombiTrans *combi_rail_L = new TGeoCombiTrans(-241,-18.25, 0,rot_90x);//y=
     combi_rail_L->SetName("combi_rail_L");
     combi_rail_L->RegisterYourself();
    
    ///////// trasl   L and R
     TGeoTranslation *tr_sr_l = new TGeoTranslation("tr_sr_l",-150.1,0,0);  //
     tr_sr_l->RegisterYourself();      

     TGeoTranslation *tr_sr_r = new TGeoTranslation("tr_sr_r",150.1,0,0);  //
     tr_sr_r->RegisterYourself();      

     ///////////////////////  shape for rail R ////////
     auto *s_RR_1box =new TGeoBBox("S_RR_1BOX", x_RR_1box/2,y_RR_1box/2,z_RR_1box/2);

     auto *s_qdi_Rbox =new TGeoBBox("S_QDI_RBOX", x_qdi_Rbox/2,y_qdi_Rbox/2,z_qdi_Rbox/2);

     auto *s_ir_hole=new TGeoTube("S_ir_HOLE",radin_ir_rail,radout_ir_rail,high_ir_rail/2);
   //  auto *s_cc_hole=new TGeoTube("S_CC_HOLE",radin_cc_rail,radout_cc_rail,high_cc_rail/2);

      ////////////////////// composite shape for rail R  //////////////////////
     auto * RR_Shape_0 = new TGeoCompositeShape("RR_Shape_0","S_RR_1BOX:tr_RR_1box+ S_XTRU_RR1  + S_XTRU_RR2 +S_XTRU3_RR- S_QDI_BOX:combi_qdi + S_ir_HOLE:combi_ir_RR +S_ir_HOLE:combi_ir2_RR     "); //-S_RR_1BOX:tr_RL_1box- S_b_HOLE:tr_b_RR -S_CC_HOLE:combi_cc2_RR

 
           // JOIN only for show L and R parts

    auto * rail_L_R_Shape = new TGeoCompositeShape("RAIL_L_R_Shape","  RL_Shape_0:combi_rail_L + RR_Shape_0:combi_rail_R"); 
      //////////////////////////////////////
     
      TGeoVolume *rail_L_R_vol0 = new TGeoVolume("RAIL_L_R_VOL0",rail_L_R_Shape,kMedAlu);

    rail_L_R->AddNode(rail_L_R_vol0,2); 
 
   /// ///__8th___ pice support rail MB  /////////////////////////////////////////

   // piece  -\_ -----0000------

    auto *sup_rail_MBL = new TGeoVolumeAssembly("sup_rail_MBL");

    /////////////////VARIAb
      /// variable

     TGeoXtru *xtru_MBL_0 = new TGeoXtru(2);
     xtru_RL1->SetName("S_XTRU_MBL_0");
     //TGeoVolume * v_MBL_0 = gGeoManager->MakeXtru("V_MBL_0",Al,2);
     //TGeoXtru *xtru_MBL_0 = (TGeoXtru*)v_MBL_0->GetShape();
           // vertices a,b,c,d,e,f,g,h
     Double_t x[8]={0.,0,61.,315.5,345.5,345.5,319.46,64.96};
     Double_t y[8]={-4,4,4,130,130,122,122,-4};

     xtru_MBL_0->DefinePolygon(8,x,y);
     xtru_MBL_0->DefineSection(0,-4.,0,0,1);//(plane, -zplane/ +zplane,x0,y0,(x/y))
     xtru_MBL_0->DefineSection(1,4,0,0,1);

     TGeoRotation *rot1_MBL_0 = new TGeoRotation("rot1_MBL_0", -90,-90,90);
     rot1_MBL_0->RegisterYourself();

    /////////////////////////////////  quit box in diag/////////////////
     Double_t x_mb_box=8;  //dx=4
     Double_t y_mb_box=8;  // dy=4
     Double_t z_mb_box=8.1;  // dz=4     to quit
     TGeoTranslation *tr_mb_box= new TGeoTranslation("tr_mb_box",240.5,95.5,0);// 240.5
     tr_mb_box->RegisterYourself();
    /// circular hole_1mbl. diameter=3.5 H9  
     Double_t radin_1mb =0.; 
     Double_t radout_1mb=1.75;// diameter 3.5
     Double_t high_1mb=28.25;///  dh=+/- 4
     TGeoTranslation *tr1_mb= new TGeoTranslation("tr1_mb",184.8,61,0); //right
     tr1_mb->RegisterYourself();

     TGeoTranslation *tr2_mb= new TGeoTranslation("tr2_mb",241.5,89,0); //right
     tr2_mb->RegisterYourself();

      // ....
       ///circular hole_2mbl inclined and hole-up. diameter=M3 (3 mm)) prof , tar:8mm 
     Double_t radin_2mb =0.; 
     Double_t radout_2mb=1.5;// diameter 3
     Double_t high_2mb=8.2;///  dh=+/- 4

     TGeoRotation *rot_hole2_MBL = new TGeoRotation("rot_hole2_MBL", 0,90,0);
     rot_hole2_MBL->RegisterYourself();
      
     TGeoTranslation *tr_mbl = new TGeoTranslation("tr_mbl",-75,0,0);  //
     tr_mbl->RegisterYourself();      

     TGeoTranslation *tr_mbr = new TGeoTranslation("tr_mbr",75,0,0);  //
     tr_mbr->RegisterYourself(); 

      ///hole up ||--  hup
         //TGeoRotation *rot_RL1hole = new TGeoRotation("rot_RL1hole", 0,0,0);
      //rot_RL1hole->SetName("rot_RL1hole");
      //rot_RL1hole->RegisterYourself();
     TGeoCombiTrans *combi_hup_mb = new TGeoCombiTrans(325,126, 0,rot_90x);//y=
     combi_hup_mb->SetName("combi_hup_mb");
     combi_hup_mb->RegisterYourself();
       
 /////////////////////// shape for rail MB  /////////
     auto *s_mb_box =new TGeoBBox("S_MB_BOX", x_mb_box/2,y_mb_box/2,z_mb_box/2);
     auto *s_hole_1mbl=new TGeoTube("S_HOLE_1MBL",radin_1mb,radout_1mb,high_1mb/2); // d3.5
     auto *s_hole_2mbl=new TGeoTube("S_HOLE_2MBL",radin_2mb,radout_2mb,high_2mb/2); //d3
 
    ////////////////////// composite shape for rail_MB  R + L //////////////////////
     auto * MB_Shape_0 = new TGeoCompositeShape("MB_Shape_0","  S_XTRU_MBL_0 - S_MB_BOX:tr_mb_box - S_HOLE_1MBL:tr1_mb + S_HOLE_1MBL:tr2_mb -S_HOLE_2MBL:combi_hup_mb  "); 
     auto * MB_Shape_1 = new TGeoCompositeShape("MB_Shape_1","  MB_Shape_0:rot1_MBL_0  - S_HOLE_2MBL  "); // one piece "completed"
    // left and right-->
     auto * MB_Shape_3 = new TGeoCompositeShape("MB_Shape_3"," MB_Shape_1:tr_mbl +  MB_Shape_1:tr_mbr ");  
    
  /////////
      TGeoVolume *sup_rail_MBL_vol0 = new TGeoVolume("SUPPORT_MBL_VOL0",MB_Shape_0,kMedAlu);
      TGeoVolume *sup_rail_MBL_vol1 = new TGeoVolume("SUPPORT_MBL_VOL1",MB_Shape_3,kMedAlu);

 
     sup_rail_MBL->AddNode(sup_rail_MBL_vol1,1); 
    
     ////////////////////////////////
     TGeoRotation  *rot_halfR = new TGeoRotation("rot_halfR", 180,180,0);//half0_R
     rot_halfR->RegisterYourself();
                
  /////////////////////////////////////////////////  =|=|=
                           
      auto *stair = new TGeoVolumeAssembly("stair");

     stair->AddNode(sup_rail_MBL,1,new TGeoTranslation(0,0,0));
     stair->AddNode(Cross_mft,2,new TGeoTranslation(0,0,-45.5));
     stair->AddNode(Cross_mb0,3,new TGeoTranslation(0,16.5,-95.5));
     stair->AddNode(Cross_mb0,4,new TGeoTranslation(0,61,-184.8));
     stair->AddNode(Cross_mft,6,new TGeoTranslation(0,100,-264));
     stair->AddNode(Cross_mft,7,  new TGeoTranslation(0,116,-298.5));
    
        ///--HalfConeVolume->AddNode(rail_L, 0); //
        ////-- HalfConeVolume->AddNode(rail_L, 1); //

         //////// stair->Draw();
       /////////////////////////////
       // Cross_mb0->Draw("ogl");
       // Cross_mft->Draw("ogl");

       // Fra_front->Draw("ogl");
       // base->Draw("ogl");  //"ogl"
       //   midle->Draw("ogl");
       // rail_L->Draw("ogl");

      //  rail_R->Draw("ogl");
      //  rail_L_R->Draw("ogl");
      //     sup_rail_MBL->Draw("ogl");
 
      //// HalfConeVolume->Draw();

      //HalfSupportVolume->AddNode(sup_rail_MBL_vol1, 2);

    //  HalfSupportVolume->AddNode(stair,1,new TGeoTranslation(0,-288,0));
  //ok.    HalfSupportVolume->AddNode(base,2,new TGeoTranslation(0,0,0));
      HalfSupportVolume->AddNode(rail_L_R,3,new TGeoTranslation(0,-69,6)); // R&L
    //  HalfSupportVolume->AddNode(Fra_front,4,new TGeoTranslation(-71,-162,-325)); //
    //  HalfSupportVolume->AddNode(Fra_front,5,new TGeoCombiTrans(71,-162,-325,rot_halfR));//     
    //  HalfSupportVolume->AddNode(midle,6,new TGeoTranslation(0,-76.25,-241.5)); //
    //  HalfSupportVolume->AddNode(midle,7, new TGeoCombiTrans(0,-76.25,-241.5,rot_halfR)); 

       ////HalfSupportVolume->Draw("ogl");




////////////
   
  return HalfSupportVolume;

 


}
