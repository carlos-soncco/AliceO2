// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file HalfCone.cxx
/// \brief Class building geometry of one half of one MFT half-cone
/// \author sbest@pucp.pe, eric.endress@gmx.de, franck.manso@clermont.in2p3.fr csoncco@pucp.pe
/// \date 15/12/2016

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

#include "MFTBase/HalfCone.h"

using namespace o2::MFT;

ClassImp(o2::MFT::HalfCone);

//_____________________________________________________________________________
HalfCone::HalfCone() : TNamed(), mHalfCone(nullptr)
{

  // default constructor
}

//_____________________________________________________________________________
HalfCone::~HalfCone() = default;

//_____________________________________________________________________________
TGeoVolumeAssembly* HalfCone::createHalfCone(Int_t half)
{

     auto *HalfSupportVolume = new TGeoVolumeAssembly("HalfSupportVolume");
  
      TGeoMedium *kMedAlu = gGeoManager->GetMedium("MFT_Alu$");
      //--- define some materials
      TGeoMaterial *matVacuum = new TGeoMaterial("Vacuum", 0,0,0);
      TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);
    //   //--- define some media
      TGeoMedium *Vacuum = new TGeoMedium("Vacuum",1, matVacuum);
      TGeoMedium *Al = new TGeoMedium("Root Material",2, matAl);
   /////Rotation  ////
            
    TGeoRotation   *rot1 = new TGeoRotation("rot1", 180,-180,0); 

      rot1->RegisterYourself();
      TGeoRotation   *rot2 = new TGeoRotation("rot2", 90,-90,0);

      rot2->RegisterYourself();

      TGeoRotation   *rot3 = new TGeoRotation("rot3", 0,90,0);
      rot3->RegisterYourself();

      TGeoRotation  *rot_90x = new TGeoRotation("rot_90x", 0,-90,0); // half0

      rot_90x->RegisterYourself();

      TGeoRotation   *rot_base = new TGeoRotation("rot_base", 180,180,0); // rail_r
      rot_base->RegisterYourself();
      

      TGeoCombiTrans *combi1 = new TGeoCombiTrans(0, -10.3, 1.29, rot1); //y=-10.80 belt
      combi1->RegisterYourself();
      TGeoCombiTrans *combi2 = new TGeoCombiTrans(-16.8, 0., 0., rot2);
       
       combi2->RegisterYourself();
      TGeoRotation *r0 = new TGeoRotation("r0",10.,0.,0.);
      r0->RegisterYourself();

    ///// 1st piece  ---------  Cross_beam_MB0----------------------///

       auto *Cross_mb0 = new TGeoVolumeAssembly("Cross_mb0");
      /// variables
       // rectangular box
      Double_t x_box_mb0 =14.4;  // dx= 7.2 cm
      Double_t y_box_mb0 =0.6;
      Double_t z_box_mb0 =0.6;

      ///// holes tub  1hole tranversal
      Double_t radin_1hole_mb0=0.;
      Double_t radout_1hole_mb0=0.175;// diameter 3.5 H9  (0.35cm)
      Double_t high_1hole_mb0=0.7;///

      TGeoRotation *rot_1hole_mb0 = new TGeoRotation("rot_1hole_mb0", 0,90,0);
      rot_1hole_mb0->RegisterYourself();
        /// h= hole
      TGeoCombiTrans *acombi_1h_mb0 = new TGeoCombiTrans(5.2,0, 0,rot_1hole_mb0);
      acombi_1h_mb0->SetName("acombi_1h_mb0");
      acombi_1h_mb0->RegisterYourself();
      TGeoCombiTrans *bcombi_1h_mb0 = new TGeoCombiTrans(-5.2,0, 0,rot_1hole_mb0);//y=
      bcombi_1h_mb0->SetName("bcombi_1h_mb0");
      bcombi_1h_mb0->RegisterYourself();

      ///// 2hole coaxial
      Double_t radin_2hole_mb0=0.;
      Double_t radout_2hole_mb0=0.15;// diameter M3
      Double_t high_2hole_mb0=1.2;/// 12

      TGeoRotation *rot_2hole_mb0 = new TGeoRotation("rot_2hole_mb0", 90,90,0);
      rot_2hole_mb0->SetName("rot_2hole_mb0");
      rot_2hole_mb0->RegisterYourself();

      TGeoCombiTrans *combi_2hole_mb0 = new TGeoCombiTrans(6.7,0, 0,rot_2hole_mb0);
      combi_2hole_mb0->SetName("combi_2hole_mb0");
      combi_2hole_mb0->RegisterYourself();

      TGeoCombiTrans *combi_2hole_mb0_b = new TGeoCombiTrans(-6.7,0, 0,rot_2hole_mb0);//y=
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

    ///// 2nd piece   ------ cross beam--MFT---------(cbeam)-----

    auto *Cross_mft = new TGeoVolumeAssembly("Cross_mft");
      /// variables
    
      // using the same "box" of the 1 piece
    ////////// 2hole coaxial
      Double_t radin_hole_cbeam=0.;
      Double_t radout_hole_cbeam=0.15;// diameter M3
      Double_t high_hole_cbeam=0.91;///

     // same rotation in tub aximatAl "rot_2hole_mb0"

      TGeoCombiTrans *combi_hole_1cbeam = new TGeoCombiTrans(6.8,0, 0,rot_2hole_mb0);
      combi_hole_1cbeam->SetName("combi_hole_1cbeam");
      combi_hole_1cbeam->RegisterYourself();

      TGeoCombiTrans *combi_hole_2cbeam = new TGeoCombiTrans(-6.8,0, 0,rot_2hole_mb0);
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

   
    ////3th piece  -------- Framework front ------------

    auto *Fra_front = new TGeoVolumeAssembly("Fra_front");
         auto *Fra_front_L = new TGeoVolumeAssembly("Fra_front_L");
         auto *Fra_front_R = new TGeoVolumeAssembly("Fra_front_R");
     
      Double_t x_box_up =0.6;  // cm
      Double_t y_box_up =0.605;
      Double_t z_box_up =2.84;
        //hole up //
      Double_t dia_tub_up=0.35;
      Double_t high_tub_up=0.65;
      //hole down////
      Double_t dia_tubdown =0.35;
      Double_t high_tubdown =0.68; 
      // 
      Double_t x_boxA_down=0.8;
      Double_t y_boxA_down=0.6;
      Double_t z_boxA_down=0.6;
      //
      Double_t x_boxB_down=0.6;
      Double_t y_boxB_down=0.605;
      Double_t z_boxB_down=1.26; // 12.6
      //seg tub
      Double_t radin_segtub=16.9;
      Double_t radout_segtub=17.5;
      Double_t high_segtub=0.6;
      Double_t ang_in_segtub=212.1;
      Double_t ang_fin_segtub=241.92; //  

      /////// trans. rot.
      TGeoCombiTrans *combi_3a = new TGeoCombiTrans(-7.4, 0, 8.975,rot2);
      combi_3a->SetName("combi_3a");
      combi_3a->RegisterYourself();

      TGeoTranslation *tr1_up = new TGeoTranslation("tr1_up",-7.4,0,8.28);  //
     
      tr1_up->RegisterYourself(); 
     
      TGeoTranslation *tr1_tub1 = new TGeoTranslation("tr1_tub1",0,0.,3.075);
      tr1_tub1->RegisterYourself();

      TGeoCombiTrans *combi_3b = new TGeoCombiTrans(7.118, 0, 16.16,rot3);
      combi_3b->SetName("combi_3b");
      combi_3b->RegisterYourself();
   
      TGeoTranslation *tr_2_box = new TGeoTranslation("tr_2_box",-0.4,0,0.7);  //
      tr_2_box->RegisterYourself();

      TGeoTranslation *tr3_box = new TGeoTranslation("tr3_box",-1.1,0,0.63);  //
      tr3_box->RegisterYourself();

      TGeoTranslation *tr_tubdown = new TGeoTranslation("tr_tubdown",-0.4,0,0.7);  //
      tr_tubdown->RegisterYourself();

      /////// shape for framewor front
     
      TGeoShape *s_box_up = new TGeoBBox("BOX_UP", x_box_up/2,y_box_up/2,z_box_up/2);

      TGeoShape *s_tub_up=new TGeoTube("TUB_UP",0.,dia_tub_up/2,high_tub_up/2); //
      TGeoShape *s_seg_tub=new TGeoTubeSeg("SEG_TUB",radin_segtub,radout_segtub,high_segtub/2,ang_in_segtub,ang_fin_segtub);//r_in,r_out,dZ,ang,ang

      TGeoShape *s_boxB_down = new TGeoBBox("BOXB_DOWN", x_boxB_down/2,y_boxB_down/2,z_boxB_down/2);
        
      TGeoShape * s_boxA_down = new TGeoBBox("BOXA_DOWN", x_boxA_down/2,y_boxA_down/2,z_boxA_down/2);
    
      TGeoShape *s_tubdown=new TGeoTube("S_TUBDOWN",0.,dia_tubdown/2,high_tubdown/2); 
      
     //Composite shapes for Fra_front
      auto * Fra_front_Shape_0 = new TGeoCompositeShape("Fra_front_Shape_0","BOX_UP:tr1_up +SEG_TUB:combi_3b+ BOXB_DOWN:tr3_box + BOXA_DOWN:tr_2_box"); 

      auto * Fra_front_Shape_1 = new TGeoCompositeShape("Fra_front_Shape_1","Fra_front_Shape_0 - S_TUBDOWN:tr_tubdown -TUB_UP:combi_3a");
      auto * Fra_front_Shape_2 = new TGeoCompositeShape("Fra_front_Shape_2","Fra_front_Shape_1:rot_90x -TUB_UP:combi_3a"); //-

          TGeoRotation  *rot_z180x90 = new TGeoRotation("rot_z180x90", 180,90,0);//half0_R
     rot_z180x90->RegisterYourself();
   
  
   TGeoRotation  *rot_halfR = new TGeoRotation("rot_halfR", 180,180,0);//half0_R
     rot_halfR->RegisterYourself();
     TGeoCombiTrans *combi_front_L = new TGeoCombiTrans(-7.1, -16.2, 32.5+0.675,rot_90x);//x=7.35, y=0, z=15.79
      combi_front_L->SetName("combi_front_L");
      combi_front_L->RegisterYourself();

     TGeoCombiTrans *combi_front_R = new TGeoCombiTrans(7.1, -16.2, 32.5+0.675,rot_z180x90);//x=7.35, y=0, z=15.79
      combi_front_R->SetName("combi_front_R");
      combi_front_R->RegisterYourself();


      auto * Fra_front_Shape_3 = new TGeoCompositeShape("Fra_front_Shape_3","Fra_front_Shape_2:rot_halfR  "); 

      auto * Fra_front_Volume = new TGeoVolume("Fra_front_Volume",Fra_front_Shape_1,kMedAlu);

        Fra_front_L->AddNode(Fra_front_Volume,1,combi_front_L);
        Fra_front_R->AddNode(Fra_front_Volume,1,combi_front_R);
 
      Fra_front->AddNode(Fra_front_L,1);  
      Fra_front->AddNode(Fra_front_R,2);
  
 ///////////////////////////////////////////////-----

     //4th piece ------------------"BASE" framework half support ------

     auto *base = new TGeoVolumeAssembly("base");

   //seg tub  disc
      Double_t radin_disc=23.6;
      Double_t radout_disc=30.3;
      Double_t high_disc=1.35;
      Double_t ang_in_disc=180;
      Double_t ang_fin_disc=360; 

    ///// holes tub  1hole tranversal o3.5
      Double_t radin_holeB=0.;
      Double_t radout_holeB=0.175;// diameter 3.5 H11
      Double_t high_holeB=1.5;///
      TGeoTranslation *tr1_holeB = new TGeoTranslation("tr1_holeB",-7.5,-28.8,0);
      tr1_holeB->RegisterYourself();

      TGeoTranslation *tr2_holeB = new TGeoTranslation("tr2_holeB",7.5,-28.8,0);
      tr2_holeB->RegisterYourself();

     //box 1
      Double_t x_1box=61.0;
      Double_t y_1box=13.0;
      Double_t z_1box=1.4;
      //box 2
      Double_t x_2box=51.2;
      Double_t y_2box=14.6;
      Double_t z_2box=1.4;
      //box 3
      Double_t x_3box=45.1;
      Double_t y_3box=23.812;
      Double_t z_3box=1.4;
     //seg tub hole 
      Double_t radin_1hole=29.3;
      Double_t radout_1hole=30.3;
      Double_t high_1hole=1.4;
      Double_t ang_in_1hole=205;
      Double_t ang_fin_1hole=225; 
      //seg tub 2 hole 
      Double_t radin_2hole=23.0;
      Double_t radout_2hole=25.5;
      Double_t high_2hole=1.4;
      Double_t ang_in_2hole=207.83;
      Double_t ang_fin_2hole=249.998; 
      //seg tub 3 ARC central xy  SEG_3ARC U
      Double_t radin_3hole=25.5;
      Double_t radout_3hole=27.5;//304
      Double_t high_3hole=1.35;
      Double_t ang_in_3hole=255.253;
      Double_t ang_fin_3hole=284.746; //284.746
    //// hole central down |_|   since x=-70 to 0
      Double_t xc_box= 7.0 ;
      Double_t yc_box= 5.772;
      Double_t zc_box= 1.4;
     
      TGeoTranslation *tr_cbox = new TGeoTranslation("tr_cbox",-xc_box/2,-radout_disc+0.888,0);  //
      tr_cbox->RegisterYourself(); 
     //box 4 lamine 1
      Double_t x_labox=60.0;
      Double_t y_labox=30.3;
      Double_t z_labox=0.305;
      TGeoTranslation *tr_la = new TGeoTranslation("tr_la",0,-y_labox/2-9.3,high_disc/2);  //
      tr_la->RegisterYourself();

      /////box 5   lamin 2
      Double_t x_2labox=51.2;
      Double_t y_2labox=2.8; //C-B
      Double_t z_2labox=0.303;
      TGeoTranslation *tr_2la = new TGeoTranslation("tr_2la",0,-8.1,high_disc/2);  //
      tr_2la->RegisterYourself();

      ///////// circular border C //--SEG_BORD   ----
      //seg tub 3 xy 
      Double_t radin_bord=0.5;
      Double_t radout_bord=0.9;//304
      Double_t high_bord=1.355;///13.5
      Double_t ang_in_bord=0;
      Double_t ang_fin_bord=90; 
      /////TGeoRotation *rot_bord1 = new TGeoRotation("rot_bord1", ang_in_1hole +0.0167,0,0);
      TGeoRotation *rot1_bord1 = new TGeoRotation("rot1_bord1", 14.8,0,0);
      rot1_bord1->RegisterYourself();
      TGeoCombiTrans *combi_bord1 = new TGeoCombiTrans(-26.7995,-13.0215, 0,rot1_bord1);//y=
      combi_bord1->SetName("combi_bord1");
      combi_bord1->RegisterYourself();

      TGeoRotation *rot2_bord1 = new TGeoRotation("rot2_bord1", -50,0,0);
      rot2_bord1->RegisterYourself();
      TGeoCombiTrans *combi2_bord1 = new TGeoCombiTrans(-21.3795,-20.7636, 0,rot2_bord1);//y=
      combi2_bord1->SetName("combi2_bord1");
      combi2_bord1->RegisterYourself();
  //////// /__0/
      TGeoRotation *rot1_bord2 = new TGeoRotation("rot1_bord2", 250,0,0);
      rot1_bord2->RegisterYourself();
      TGeoCombiTrans *combi1_bord2 = new TGeoCombiTrans(-9.0527,-23.3006, 0,rot1_bord2);//y=
      combi1_bord2->SetName("combi1_bord2");
      combi1_bord2->RegisterYourself();
     /////// |°____°|
      TGeoRotation *rot_cent_bord = new TGeoRotation("rot_cent_bord", 90,0,0);
      rot_cent_bord->RegisterYourself();
      TGeoCombiTrans *combi_cent_bord = new TGeoCombiTrans(-6.5,-27.094, 0,rot_cent_bord);//y=
      combi_cent_bord->SetName("combi_cent_bord");
      combi_cent_bord->RegisterYourself();
   ///////////////// box tonge
      Double_t x_tong=2.0;
      Double_t y_tong=2.81;
      Double_t z_tong=1.35;
      TGeoTranslation *tr_tong= new TGeoTranslation("tr_tong",0,-28.6,0);  //
      tr_tong->RegisterYourself();
     //// circular central hole1 to conexion with other parts 
      Double_t radin_hole1=0;
      Double_t radout_hole1=0.4;//
      Double_t high_hole1=1.36;///
      TGeoTranslation *tr_hole1= new TGeoTranslation("tr_hole1",0,-28.0,0);  //tonge
      tr_hole1->RegisterYourself();
       
      TGeoTranslation *tr2_hole1= new TGeoTranslation("tr2_hole1",-26.5,-8.5,0); //left
      tr2_hole1->RegisterYourself();
       
      TGeoTranslation *tr3_hole1= new TGeoTranslation("tr3_hole1",26.5,-8.5,0);  //right
      tr3_hole1->RegisterYourself();
  /////////////////////////////////////////
       /// circular hole2 ; hole2 r=6.7
      Double_t radin_hole2=0;
      Double_t radout_hole2=0.335;// diameter 6.7
      Double_t high_hole2=1.36;///13.5
      TGeoTranslation *tr1_hole2= new TGeoTranslation("tr1_hole2",-28.0,-8.5,0);  //
      tr1_hole2->RegisterYourself();
       
      TGeoTranslation *tr2_hole2= new TGeoTranslation("tr2_hole2",28.0,-8.5,0);  //
      tr2_hole2->RegisterYourself();


//////////// hole "0" two tubs together  
      Double_t radin_T1=0.325; // diam 0.65cm
      Double_t radout_T1=0.55;// dia 1.1   
      Double_t high_T1=1.2;///  dz 6
       
      Double_t radin_T2=0;
      Double_t radout_T2=1.1;//
      Double_t high_T2=1.2;///  dz 6
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
   //base->AddNode(base4_Volume,2); 
 //////////////////////////////////////////


   ///5th piece MIDLE ------ Framework midle--------------------

     auto *midle = new TGeoVolumeAssembly("Midle");

        auto *midle_L = new TGeoVolumeAssembly("Midle_L");
        auto *midle_R = new TGeoVolumeAssembly("Midle_R");
        
  
       ////box up to quit and to join
       Double_t x_midle=0.8; //dx=4
       Double_t y_midle=3.495; //y=34.9
       Double_t z_midle=0.62;   //z=6
      // tr1 to join with arc
       TGeoTranslation *tr1_midle_box= new TGeoTranslation("tr1_midle_box",-14.4,-0.745,0);  //-152,-17.45,0
       tr1_midle_box->RegisterYourself();
       //tr2 to quiet
       TGeoTranslation *tr2_midle_box= new TGeoTranslation("tr2_midle_box",-15.2,-0.745,0);  //-152,-17.45,0
       tr2_midle_box->RegisterYourself();

    ///////////////////////////
      ////box down_1
       Double_t x_midle_d1box=0.4; //dx=4
       Double_t y_midle_d1box=0.28;
       Double_t z_midle_d1box=0.66;
       TGeoTranslation *tr_midle_d1box= new TGeoTranslation("tr_midle_d1box",-7.3,-11.96,0.);  //81
       tr_midle_d1box->RegisterYourself();

     ///////////////////////////
      ////box down_2
      Double_t x_midle_d2box=0.8; //dx=4
      Double_t y_midle_d2box=1.0;
      Double_t z_midle_d2box=0.66; // 6.5 -> 6.6 to quit
      TGeoTranslation *tr_midle_d2box= new TGeoTranslation("tr_midle_d2box",-7.5,-12.6249,0);  //81
      tr_midle_d2box->RegisterYourself();
  
      ///////////////////
       //arc circ part 
      Double_t radin_midle=14.0;
      Double_t radout_midle=15.0;//
      Double_t high_midle=0.6;///
      Double_t ang_in_midle=180;
      Double_t ang_fin_midle=238.21;  //alfa=57.60   .(first 237.31)

      /////////////////////////////////////////
       /// circular hole1 ; hole_midle d=3.5
      Double_t radin_mid_1hole=0.;
      Double_t radout_mid_1hole=0.175;// diameter 3.5
      Double_t high_mid_1hole=1.5;/// 2.4

      TGeoRotation *rot_mid_1hole = new TGeoRotation("rot_mid_1hole", 90,90,0);
      rot_mid_1hole->RegisterYourself();
      TGeoCombiTrans *combi_mid_1tubhole = new TGeoCombiTrans(-14.2,0.325, 0,rot_mid_1hole);//
      combi_mid_1tubhole->SetName("combi_mid_1tubhole");
      combi_mid_1tubhole->RegisterYourself();

      /////////////////////////////////////////
      /// circular hole2 ; hole_midle d=3
      Double_t radin_mid_2hole=0.;
      Double_t radout_mid_2hole=0.15;// diameter 3
      Double_t high_mid_2hole=1.8;///
    
      TGeoCombiTrans *combi_mid_2tubhole = new TGeoCombiTrans(-7.7,-12.355, 0,rot_mid_1hole);//x=81
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

    TGeoRotation  *rot_midlez = new TGeoRotation("rot_midley", 180,180,0);//half0_R
     rot_midlez->RegisterYourself();
     TGeoCombiTrans *combi_midle_L = new TGeoCombiTrans(0, -7.625, 24.15+0.675,rot_90x);//x=7.35, y=0, z=15.79  -- 0,-7.625,24.15+0.675-80)
      combi_midle_L->SetName("combi_midle_L");
      combi_midle_L->RegisterYourself();
   
       TGeoTranslation *tr_midle_L= new TGeoTranslation("tr_midle_L",0,-7.625,24.15+0.675);  //-152,-17.45,0
       tr_midle_L->RegisterYourself();

     TGeoCombiTrans *combi_midle_R = new TGeoCombiTrans(0, -7.625,24.15+0.675,rot_midlez);//x=7.35, y=0, z=15.79
      combi_midle_R->SetName("combi_midle_R");
      combi_midle_R->RegisterYourself();
   
     auto * midle_Volume = new TGeoVolume("midle_Volume",midle_Shape_1,kMedAlu);

     midle_L->AddNode(midle_Volume,1,tr_midle_L);
     midle_R->AddNode(midle_Volume,1,combi_midle_R);

     //midle->AddNode(midle_Volume,1); 
     midle->AddNode(midle_L,1);
     midle->AddNode(midle_R,2);  

  //////////////////////////////////////new piece ... _/   \_  
      /// Support_rail_L & Support_rail_R

   auto *rail_L_R = new TGeoVolumeAssembly("rail_L_R");

      //6 piece RAIL LEFT --RL---0000------
     auto *rail_L = new TGeoVolumeAssembly("rail_L");

    /////////////////VARIAb
            ////box down_2
     Double_t x_RL_1box=3.0; //dx=15
     Double_t y_RL_1box=1.21;  // dy=6, -dy=6
     Double_t z_RL_1box=0.8; // dz=4     to quit
     TGeoTranslation *tr_RL_1box= new TGeoTranslation(0,y_RL_1box/2,1.825);  //81
     tr_RL_1box->SetName("tr_RL_1box");
     tr_RL_1box->RegisterYourself();
     //////////////////////////////7

     TGeoXtru *xtru_RL1 = new TGeoXtru(2);
     xtru_RL1->SetName("S_XTRU_RL1");
        
     Double_t x_RL1[5]={-1.5,1.5,0.5,0.5,-1.5 };//93,93,73,73,-15};//vertices
     Double_t y_RL1[5]={1.2,1.2,2.2,8.2,8.2};//357.5,357.5,250.78,145.91};
     xtru_RL1->DefinePolygon(5,x_RL1,y_RL1);
     xtru_RL1->DefineSection(0,-2.225,0.,0.,1);  //(plane,-zplane/ +zplane, x0, y0,(x/y))
     xtru_RL1->DefineSection(1,2.225,0.,0.,1);
    ////////////// \//
       
     TGeoXtru *xtru_RL2 = new TGeoXtru(2);
     xtru_RL2->SetName("S_XTRU_RL2");
         
     Double_t x_RL2[8]={-1.5,0.5,0.5,9.3,9.3,7.3,7.3,-1.5};   //vertices
     Double_t y_RL2[8]={8.2,8.2,13.863,24.35,35.75,35.75,25.078,14.591};

     xtru_RL2->DefinePolygon(8,x_RL2,y_RL2);

     xtru_RL2->DefineSection(0,0.776,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru_RL2->DefineSection(1,2.225 ,0,0,1);

/////////////////////////  ////////////////////
     ////box knee
     Double_t x_RL_kneebox=1.5; //dx=7.5
     Double_t y_RL_kneebox=3.5;  // dy=17.5
     Double_t z_RL_kneebox=1.5; // dz=7.5     to quit
     TGeoTranslation *tr_RL_kneebox= new TGeoTranslation(0,0,0);  //81 x =-2.5, y=145.91
     tr_RL_kneebox->SetName("tr_RL_kneebox");
     tr_RL_kneebox->RegisterYourself();

     TGeoRotation *rot_knee = new TGeoRotation("rot_knee", -40,0,0);
     rot_knee->SetName("rot_knee");
     rot_knee->RegisterYourself();
     TGeoCombiTrans *combi_knee = new TGeoCombiTrans(0.96,1.75+0.81864, 0,rot_knee);//y=
     combi_knee->SetName("combi_knee");
     combi_knee->RegisterYourself();
   /////////////////////////////////  quit diagona-> qdi/////////////////
     Double_t x_qdi_box=3.1; //
     Double_t y_qdi_box=7.159;  // 
     Double_t z_qdi_box=3.005; // 

     TGeoRotation *rot_qdi = new TGeoRotation("rot_qdi", 0,24.775,0);
     rot_qdi->RegisterYourself();
     TGeoCombiTrans *combi_qdi = new TGeoCombiTrans(0,5.579, -2.087,rot_qdi);//y=
     combi_qdi->SetName("combi_qdi");
     combi_qdi->RegisterYourself();
 //////////////  knee small

     TGeoXtru *xtru3_RL = new TGeoXtru(2);
     xtru3_RL->SetName("S_XTRU3_RL");
     
     Double_t x_3RL[6]={-0.75,0.75,0.75,2.6487,1.4997,-0.75};   //vertices
     Double_t y_3RL[6]={-1.75,-1.75,1.203,3.465,4.4311,1.75};

     xtru3_RL->DefinePolygon(6,x_3RL,y_3RL);
     xtru3_RL->DefineSection(0,-0.75,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru3_RL->DefineSection(1,0.76,0,0,1);

     TGeoTranslation *tr_vol3_RL= new TGeoTranslation(-0.25,12.66,0);  //
     tr_vol3_RL->SetName("tr_vol3_RL");
     tr_vol3_RL->RegisterYourself();
/////////////////////////////////////////////
     //// circular holes  could be for rail R and L ..
     /// circular hole1_RL (a(6,22)); hole_midle d=6.5 H11
     Double_t radin_RL1hole=0.;
     Double_t radout_RL1hole=0.325;// diameter 3.5
     Double_t high_RL1hole=1.0;///

     TGeoRotation *rot_RL1hole = new TGeoRotation("rot_RL1hole", 0,0,0);
     rot_RL1hole->RegisterYourself();
     TGeoCombiTrans *combi_RL1hole = new TGeoCombiTrans(0.7,0.6, 1.85,rot_RL1hole);//y=
     combi_RL1hole->SetName("combi_RL1hole");
     combi_RL1hole->RegisterYourself();
//////////////////////////////// similar hole for R -- Join.
     /// circular hole_ir. diameter=M3 (3 mm)) prof trou:8, tar:6mm 
     Double_t radin_ir_railL=0.;
     Double_t radout_ir_railL=0.15;// diameter 0.3cm
     Double_t high_ir_railL=3.9;// 
     TGeoRotation *rot_ir_RL = new TGeoRotation("rot_ir_RL", 90,90,0);
     rot_ir_RL->RegisterYourself();
        //// in y = l_253.5 - 6. enter in (0,6,0)
     TGeoCombiTrans *combi_ir1_RL = new TGeoCombiTrans(8.62,24.75, 1.5,rot_ir_RL);
     combi_ir1_RL->SetName("combi_ir1_RL");
     combi_ir1_RL->RegisterYourself();

     TGeoCombiTrans *combi_ir2_RL = new TGeoCombiTrans(8.6,33.15, 1.5,rot_ir_RL);
     combi_ir2_RL->SetName("combi_ir2_RL");
     combi_ir2_RL->RegisterYourself();

///////////////////////  shape for Rail L geom/////////
     auto *s_RL_1box =new TGeoBBox("S_RL_1BOX", x_RL_1box/2,y_RL_1box/2,z_RL_1box/2);
     auto *s_RL_kneebox =new TGeoBBox("S_RL_KNEEBOX", x_RL_kneebox/2,y_RL_kneebox/2,z_RL_kneebox/2);
     auto *s_qdi_box =new TGeoBBox("S_QDI_BOX", x_qdi_box/2,y_qdi_box/2,z_qdi_box/2);
     auto *s_RL1hole=new TGeoTube("S_RL1HOLE",radin_RL1hole,radout_RL1hole,high_RL1hole/2);
     //-auto *s_irL_hole=new TGeoTube("S_irL_HOLE",radin_ir_railL,radout_ir_railL,high_ir_railL/2);

     
////////////////////// composite shape for rail L //////////////////////
 
     auto * RL_Shape_0 = new TGeoCompositeShape("RL_Shape_0"," S_XTRU3_RL:tr_vol3_RL +S_XTRU_RL1 + S_XTRU_RL2 +S_RL_1BOX:tr_RL_1box -S_QDI_BOX:combi_qdi");  // 

    //////////////////////////////////////-------////////

     TGeoVolume *rail_L_vol0 = new TGeoVolume("RAIL_L_VOL0",RL_Shape_0,kMedAlu);

   rail_L->AddNode(rail_L_vol0,1,new TGeoTranslation(0., 0., 1.5) );
 
    
    ////piece 7th ---------------RAIL RIGHT  ----  //////////////-----

    auto *rail_R = new TGeoVolumeAssembly("rail_R");

     Double_t x_RR_1box=3.0; //dx=15
     Double_t y_RR_1box=1.2;  // dy=6, -dy=6
     Double_t z_RR_1box=0.8; // dz=4     to quit
     TGeoTranslation *tr_RR_1box= new TGeoTranslation("tr_RR_1box",0,0.6,1.825);  //81
     tr_RR_1box->RegisterYourself();
     ////////////////////////////////7

     TGeoXtru *xtru_RR1 = new TGeoXtru(2);
     xtru_RR1->SetName("S_XTRU_RR1");
     //-TGeoVolume *vol_RR1 = gGeoManager->MakeXtru("S_XTRU_RR1",kMedAlu,2);
     //-TGeoXtru *xtru_RR1 = (TGeoXtru*)vol_RR1->GetShape();
          
     Double_t x_RR1[5]={-1.5,-0.5,-0.5,1.5,1.5 };//C,D,K,L,C' //vertices
     Double_t y_RR1[5]={1.2,2.2,8.2,8.2,1.2};//357.5,357.5,250.78,145.91};

     xtru_RR1->DefinePolygon(5,x_RR1,y_RR1);
     xtru_RR1->DefineSection(0,-2.225,0,0,1);  //(plane,-zplane/ +zplane, x0, y0,(x/y))
     xtru_RR1->DefineSection(1,2.225,0,0,1);

    ////////////// \//////////

     TGeoXtru *xtru_RR2 = new TGeoXtru(2);
     xtru_RR2->SetName("S_XTRU_RR2");
     //-TGeoVolume *vol_RR2 = gGeoManager->MakeXtru("S_XTRU_RR2",Al,2);
     //-TGeoXtru *xtru_RR2 = (TGeoXtru*)vol_RR2->GetShape();

     Double_t x_RR2[8]={-0.5,-0.5,-9.3,-9.3,-7.3,-7.3,1.5,1.5};//K,E,F,G,H,I,J,L//vertices
     Double_t y_RR2[8]={8.2,13.863,24.35,35.75,35.75,25.078,14.591,8.2};

     xtru_RR2->DefinePolygon(8,x_RR2,y_RR2);
     xtru_RR2->DefineSection(0,0.776,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru_RR2->DefineSection(1,2.225 ,0,0,1);

     //////////////  knee (small)

     TGeoXtru *xtru3_RR = new TGeoXtru(2);
     xtru3_RR->SetName("S_XTRU3_RR");
     
     Double_t x_3RR[6]={1.0,1.0,-1.2497,-2.2138,-0.5,-0.5};//R,Q,P,O,N.M   //vertices
     Double_t y_3RR[6]={10.91,14.41,17.0911,15.9421,13.86,10.91};

     xtru3_RR->DefinePolygon(6,x_3RR,y_3RR);
     xtru3_RR->DefineSection(0,-0.75,0,0,1);//(plane,-zplane/+zplane, x0, y0,(x/y))
     xtru3_RR->DefineSection(1,0.78,0,0,1);

     TGeoTranslation *tr_vol3_RR= new TGeoTranslation("tr_vol3_RR",-0.25,12.66,0);  //
     tr_vol3_RR->RegisterYourself();

      /////////////////////////////////  quit diagona-> qdi/////////////////
     Double_t x_qdi_Rbox=3.1; //dx=1.5
     Double_t y_qdi_Rbox=7.159;  // 
     Double_t z_qdi_Rbox=3.005; // 

     TGeoRotation *rot_Rqdi = new TGeoRotation("rot_Rqdi", 0,24.775,0);
     rot_Rqdi->RegisterYourself();
     TGeoCombiTrans *combi_Rqdi = new TGeoCombiTrans(0,5.579, -2.087,rot_Rqdi);//y=
     combi_Rqdi->SetName("combi_Rqdi");
     combi_Rqdi->RegisterYourself();

    ////////////////////___holes__/////

    /// circular hole_a. diameter=6.5 (a(6,22)); hole_midle d=6.5 H11
     Double_t radin_a_rail=0.;
     Double_t radout_a_rail=0.325;// diameter 3.5
     Double_t high_a_rail=0.82;///

     TGeoTranslation *tr_a_RR= new TGeoTranslation("tr_a_RR",-0.7,0.6,1.825);  //right
     tr_a_RR->RegisterYourself();
  //// circular hole_ir. diameter=M3 (3 mm)) prof trou:8, tar:6mm 
     Double_t radin_ir_rail=0.;
     Double_t radout_ir_rail=0.15;// diameter 3
     Double_t high_ir_rail=3.2;// 19  ---
     TGeoRotation *rot_ir_RR = new TGeoRotation("rot_ir_RR", 90,90,0);
     rot_ir_RR->RegisterYourself();
        //// in y = l_253.5 - 6. center in (0,6,0)
     TGeoCombiTrans *combi_ir_RR = new TGeoCombiTrans(-8.62,24.75, 1.5,rot_ir_RR);
     combi_ir_RR->SetName("combi_ir_RR");
     combi_ir_RR->RegisterYourself();

     TGeoCombiTrans *combi_ir2_RR = new TGeoCombiTrans(-8.6,33.15, 1.5,rot_ir_RR);
     combi_ir2_RR->SetName("combi_ir2_RR");
     combi_ir2_RR->RegisterYourself();
             
     TGeoCombiTrans *combi_rail_R = new TGeoCombiTrans(24.1,-1.825, 0,rot_90x);//y=
     combi_rail_R->SetName("combi_rail_R");
     combi_rail_R->RegisterYourself();
     TGeoCombiTrans *combi_rail_L = new TGeoCombiTrans(-24.1,-1.825, 0,rot_90x);//y=
     combi_rail_L->SetName("combi_rail_L");
     combi_rail_L->RegisterYourself();
    
    ///////// trasl   L and R
     TGeoTranslation *tr_sr_l = new TGeoTranslation("tr_sr_l",-15.01,0,0);  //
     tr_sr_l->RegisterYourself();      

     TGeoTranslation *tr_sr_r = new TGeoTranslation("tr_sr_r",15.01,0,0);  //
     tr_sr_r->RegisterYourself();      

     ///////////////////////  shape for rail R ////////
     auto *s_RR_1box =new TGeoBBox("S_RR_1BOX", x_RR_1box/2,y_RR_1box/2,z_RR_1box/2);

     auto *s_qdi_Rbox =new TGeoBBox("S_QDI_RBOX", x_qdi_Rbox/2,y_qdi_Rbox/2,z_qdi_Rbox/2);

     auto *s_ir_hole=new TGeoTube("S_ir_HOLE",radin_ir_rail,radout_ir_rail,high_ir_rail/2);

   //  auto *s_cc_hole=new TGeoTube("S_CC_HOLE",radin_cc_rail,radout_cc_rail,high_cc_rail/2);

      ////////////////////// composite shape for rail R  //////////////////////
     auto * RR_Shape_0 = new TGeoCompositeShape("RR_Shape_0","S_RR_1BOX:tr_RR_1box+ S_XTRU_RR1 + S_XTRU_RR2 +S_XTRU3_RR- S_QDI_BOX:combi_qdi ");

     //-auto * RR_Shape_0 = new TGeoCompositeShape("RR_Shape_0","S_RR_1BOX:tr_RR_1box+ S_XTRU_RR1  + S_XTRU_RR2 +S_XTRU3_RR- S_QDI_BOX:combi_qdi + S_ir_HOLE:combi_ir_RR +S_ir_HOLE:combi_ir2_RR     "); //-S_RR_1BOX:tr_RL_1box- S_b_HOLE:tr_b_RR -S_CC_HOLE:combi_cc2_RR

 
           // JOIN only for show L and R parts

    auto * rail_L_R_Shape = new TGeoCompositeShape("RAIL_L_R_Shape","  RL_Shape_0:combi_rail_L + RR_Shape_0:combi_rail_R"); 
      //////////////////////////////////////
     
      TGeoVolume *rail_L_R_vol0 = new TGeoVolume("RAIL_L_R_VOL0",rail_L_R_Shape,kMedAlu);

     TGeoRotation *rot_rLR = new TGeoRotation("rot_rLR", 180,180,0);
     rot_rLR->RegisterYourself();
     TGeoCombiTrans *combi_rLR = new TGeoCombiTrans(0,-6.9, -0.5,rot_rLR);//0,-6.9,-0.5-80
     combi_rLR->SetName("combi_rLR");
     combi_rLR->RegisterYourself();

    rail_L_R->AddNode(rail_L_R_vol0,2,combi_rLR); 
 
   ///  piece 8th -------support rail  MB ---------------  -\_ 

    auto *sup_rail_MBL = new TGeoVolumeAssembly("sup_rail_MBL");

    /////////////////VARIAb
    
     TGeoXtru *xtru_MBL_0 = new TGeoXtru(2);
     xtru_MBL_0->SetName("S_XTRU_MBL_0"); //V-MBL_0
     
           // vertices a,b,c,d,e,f,g,h
     Double_t x[8]={0.,0,6.1,31.55,34.55,34.55,31.946,6.496};
     Double_t y[8]={-0.4,0.4,0.4,13.0,13.0,12.2,12.2,-0.4};

     xtru_MBL_0->DefinePolygon(8,x,y);
     xtru_MBL_0->DefineSection(0,-0.4,0,0,1);//(plane, -zplane/ +zplane,x0,y0,(x/y))
     xtru_MBL_0->DefineSection(1,0.4,0,0,1);

     TGeoRotation *rot1_MBL_0 = new TGeoRotation("rot1_MBL_0", -90,-90,90);
     rot1_MBL_0->RegisterYourself();

    /////////////////////////////////  quit box in diag/////////////////
     Double_t x_mb_box=0.8;  //dx=4
     Double_t y_mb_box=0.8;  // dy=4
     Double_t z_mb_box=0.81;  // dz=4     to quit
     TGeoTranslation *tr_mb_box= new TGeoTranslation("tr_mb_box",24.05,9.55,0);// 240.5
     tr_mb_box->RegisterYourself();
    
     ////lateral hole-box
     Double_t x_lat_box=0.7;  //dx=0.35
     Double_t y_lat_box=1.8;  // dy=0.9
     Double_t z_lat_box=0.2;  // dz=0.1 
     TGeoTranslation *tr_lat1L_box= new TGeoTranslation("tr_lat1L_box",4.6,0,0.4);// 
     tr_lat1L_box->RegisterYourself();    
     TGeoTranslation *tr_lat2L_box= new TGeoTranslation("tr_lat2L_box",9.6,1.65,0.4);// 
     tr_lat2L_box->RegisterYourself();
     TGeoTranslation *tr_lat3L_box= new TGeoTranslation("tr_lat3L_box",18.53,6.1,0.4);// 
     tr_lat3L_box->RegisterYourself();
     TGeoTranslation *tr_lat4L_box= new TGeoTranslation("tr_lat4L_box",26.45,10,0.4);// 
     tr_lat4L_box->RegisterYourself();
     TGeoTranslation *tr_lat5L_box= new TGeoTranslation("tr_lat5L_box",29.9,11.6,0.4);// 
     tr_lat5L_box->RegisterYourself();

     TGeoTranslation *tr_lat1R_box= new TGeoTranslation("tr_lat1R_box",4.6,0,-0.4);// 
     tr_lat1R_box->RegisterYourself();    
     TGeoTranslation *tr_lat2R_box= new TGeoTranslation("tr_lat2R_box",9.6,1.65,-0.4);// 
     tr_lat2R_box->RegisterYourself();
     TGeoTranslation *tr_lat3R_box= new TGeoTranslation("tr_lat3R_box",18.53,6.1,-0.4);// 
     tr_lat3R_box->RegisterYourself();
     TGeoTranslation *tr_lat4R_box= new TGeoTranslation("tr_lat4R_box",26.45,10,-0.4);// 
     tr_lat4R_box->RegisterYourself();
     TGeoTranslation *tr_lat5R_box= new TGeoTranslation("tr_lat5R_box",29.9,11.6,-0.4);// 
     tr_lat5R_box->RegisterYourself();

    /// circular hole_1mbl. diameter=3.5 H9  
     Double_t radin_1mb =0.; 
     Double_t radout_1mb=0.175;// diameter 3.5mm _0.35 cm
     Double_t high_1mb=2.825;///  dh=+/- 4
     TGeoTranslation *tr1_mb= new TGeoTranslation("tr1_mb",18.48,6.1,0.); //right
     tr1_mb->RegisterYourself();

     TGeoTranslation *tr2_mb= new TGeoTranslation("tr2_mb",24.15,8.9,0.); //right
     tr2_mb->RegisterYourself();

      ///circular hole_2mbl inclined and hole-up.diameter=M3 (3 mm)) prof , tar:8mm 
     Double_t radin_2mb =0.; 
     Double_t radout_2mb=0.15;// diameter 0.3
     Double_t high_2mb=0.82;///  dh=+/- 4

     TGeoRotation *rot_hole2_MBL = new TGeoRotation("rot_hole2_MBL", 0,90,0);
     rot_hole2_MBL->RegisterYourself();
      
     TGeoTranslation *tr_mbl = new TGeoTranslation("tr_mbl",-7.5,0.,0.);  //
     tr_mbl->RegisterYourself();      

     TGeoTranslation *tr_mbr = new TGeoTranslation("tr_mbr",7.5,0,0);  //
     tr_mbr->RegisterYourself(); 

      ///hole up ||--  hup
         
     TGeoCombiTrans *combi_hup_mb = new TGeoCombiTrans(32.5,12.6, 0,rot_90x);//y=
     combi_hup_mb->SetName("combi_hup_mb");
     combi_hup_mb->RegisterYourself();
       
 /////////////////////// shape for rail MB  /////////
     auto *s_mb_box =new TGeoBBox("S_MB_BOX", x_mb_box/2,y_mb_box/2,z_mb_box/2);
     auto *s_hole_1mbl=new TGeoTube("S_HOLE_1MBL",radin_1mb,radout_1mb,high_1mb/2); // d3.5
     auto *s_hole_2mbl=new TGeoTube("S_HOLE_2MBL",radin_2mb,radout_2mb,high_2mb/2); //d3
     auto *s_lat_box =new TGeoBBox("S_LAT_BOX",x_lat_box/2,y_lat_box/2,z_lat_box/2);
 
    ////////////////////// composite shape for rail_MB  R + L //////////////////////

    // auto * MB_Shape_0 = new TGeoCompositeShape("MB_Shape_0","  V_MBL_0 - S_MB_BOX:tr_mb_box - S_HOLE_1MBL:tr1_mb + S_HOLE_1MBL:tr2_mb -S_HOLE_2MBL:combi_hup_mb  ");
     auto * MB_Shape_0 = new TGeoCompositeShape("MB_Shape_0","S_XTRU_MBL_0 - S_MB_BOX:tr_mb_box - S_HOLE_1MBL:tr1_mb -S_HOLE_2MBL:combi_hup_mb  "); 

    auto * MB_Shape_0L = new TGeoCompositeShape("MB_Shape_0L","MB_Shape_0 -S_LAT_BOX:tr_lat1L_box -S_LAT_BOX:tr_lat2L_box - S_LAT_BOX:tr_lat3L_box -S_LAT_BOX:tr_lat4L_box- S_LAT_BOX:tr_lat5L_box "); 

    auto * MB_Shape_0R = new TGeoCompositeShape("MB_Shape_0R","MB_Shape_0 - S_LAT_BOX:tr_lat1R_box -S_LAT_BOX:tr_lat2R_box - S_LAT_BOX:tr_lat3R_box -S_LAT_BOX:tr_lat4R_box- S_LAT_BOX:tr_lat5R_box"); 
      
     auto * MB_Shape_1L = new TGeoCompositeShape("MB_Shape_1L","MB_Shape_0L:rot1_MBL_0 - S_HOLE_2MBL"); // one piece "completed"
    // left and right-->
    auto * MB_Shape_1R = new TGeoCompositeShape("MB_Shape_1R","MB_Shape_0R:rot1_MBL_0 - S_HOLE_2MBL");
  
     auto * MB_Shape_2 = new TGeoCompositeShape("MB_Shape_2"," MB_Shape_1L:tr_mbl +  MB_Shape_1R:tr_mbr ");  
    
  /////////
     // TGeoVolume *sup_rail_MBL_vol0 = new TGeoVolume("SUPPORT_MBL_VOL0",MB_Shape_0,Al);
      TGeoVolume *sup_rail_MBL_vol = new TGeoVolume("SUPPORT_MBL_VOL",MB_Shape_2,kMedAlu);
     ////////////////////////////////
     
 
     sup_rail_MBL->AddNode(sup_rail_MBL_vol,1,rot_halfR); 
      
                
  /////////////////////////////////////////////////  =|=|=
                           
      auto *stair = new TGeoVolumeAssembly("stair");

     stair->AddNode(sup_rail_MBL,1,new TGeoTranslation(0,0-28.8,0+0.675));//
     stair->AddNode(Cross_mft,2,new TGeoTranslation(0,-28.8,4.55+0.675));
     stair->AddNode(Cross_mb0,3,new TGeoTranslation(0,1.65-28.8,9.55+0.675));
     stair->AddNode(Cross_mb0,4,new TGeoTranslation(0,6.1-28.8,18.48+0.675));
     stair->AddNode(Cross_mft,6,new TGeoTranslation(0,10.0-28.8,26.4+0.675));
     stair->AddNode(Cross_mft,7,  new TGeoTranslation(0,11.6-28.8,29.85+0.675));
    
    
       Double_t t_final_x;
       Double_t t_final_y;
       Double_t t_final_z;

       Double_t r_final_x ;
       Double_t r_final_y ;
       Double_t r_final_z ;


    if (half == 0){
                t_final_x = 0;
                t_final_y = 0;
                t_final_z = -80;

                 r_final_x =0 ;
                 r_final_y = 0;
                 r_final_z = 0;
     }
         
     if (half == 1){
                t_final_x = 0;
                t_final_y = 0;
                t_final_z = -80;

                r_final_x =0 ;
                r_final_y = 0;
                r_final_z= 180;
     }
    
    auto* t_final = new TGeoTranslation("t_final", t_final_x, t_final_y, t_final_z);
    auto* r_final = new TGeoRotation("r_final", r_final_x, r_final_y, r_final_z);
    auto* c_final = new TGeoCombiTrans(*t_final, *r_final);

 
     HalfSupportVolume->AddNode(stair,1,c_final ); //
     HalfSupportVolume->AddNode(base,2,c_final);
     HalfSupportVolume->AddNode(rail_L_R,3, c_final);// R&L 
     HalfSupportVolume->AddNode(Fra_front,4,c_final);   
     HalfSupportVolume->AddNode(midle,6,c_final); // 

////////////
 
 
  return HalfConeVolume;
}
