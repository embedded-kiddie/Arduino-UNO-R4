/*----------------------------------------------------------------------
 * 楽譜データの構造体
 *
 * typedef struct {
 *    unsigned short pitch;	// 音階
 *    unsigned short note;	// 音符長
 * } MusicScore_t;
 *
 * #define  N16     1   // 16分音符
 * #define  N8      2   //  8分音符
 * #define  N8x     3   //8.5分音符
 * #define  N4      4   //  4分音符
 * #define  N4x     6   //4.5分音符
 * #define  N2      8   //  2分音符
 * #define  N2x     12  //2.5分音符
 * #define  N0      16  //   全音符
 *
 * #define  N84     6   // N8 + N4
 * #define  N83     8   // N8 + N3
 * #define  N82     10  // N8 + N2
 * #define  N81     14  // N8 + N1
 * #define  N80     18  // N8 + N0
 *----------------------------------------------------------------------*/
/*---------------------------
 * TRUTH / T-SQUARE
 * Music by Masahiro Andoh
 *---------------------------*/

#if		1
// [Intro]
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/* 2*/	 REST,    N8  ,  NOTE_D5, N8  ,  NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,
/* 3*/	 NOTE_D6, N8  ,  NOTE_A5, N8  ,  NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,
/* 4*/	 NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,  NOTE_D6, N8  ,  NOTE_A5, N8  ,
/* 5*/	 NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,  NOTE_AS5,N8  ,  NOTE_C6, N81 ,
/* 6*/	                                                                                                 NOTE_AS5,N8  ,  NOTE_C6, N84 ,
/* 7*/	                                 NOTE_AS5,N8 ,  NOTE_C6,N84 ,                                    NOTE_C6, N8  ,  NOTE_D6, N8  ,
#endif

#if		0
// 前節の繰り返し
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/* 8*/	 REST,    N8  ,  NOTE_D5, N8  ,  NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,
/* 9*/	 NOTE_D6, N8  ,  NOTE_A5, N8  ,  NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,
/*10*/	 NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,  NOTE_D6, N8  ,  NOTE_A5, N8  ,
/*11*/	 NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,  NOTE_AS5,N8  ,  NOTE_C6, N81 ,
/*12*/	                                                                                                 NOTE_AS5,N8  ,  NOTE_C6, N84 ,
/*13*/	                                 NOTE_AS5,N8  ,  NOTE_C6, N84 ,                                  NOTE_C6, N8  ,  NOTE_D6, N8  ,
#endif

#if		1
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*14*/	 REST,    N8  ,  NOTE_D5, N8  ,  NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,
/*15*/	 NOTE_D6, N8  ,  NOTE_A5, N8  ,  NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,
/*16*/	 NOTE_F5, N8  ,  NOTE_A5, N8  ,  NOTE_F6, N8  ,  NOTE_A5, N8  ,  NOTE_E6, N8  ,  NOTE_C6, N8  ,  NOTE_D6, N8  ,  NOTE_A5, N8  ,
/*17*/	 NOTE_C6, N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_D5, N8  ,  NOTE_AS5,N8  ,  NOTE_C6, N81 ,
/*18*/	                                                                                 NOTE_AS5,N8  ,  NOTE_C6, N84 ,
/*19*/	                                                 NOTE_AS5,N8  ,  NOTE_C6, N84 ,                  NOTE_C6, N8  ,  NOTE_A5, N81 ,
/*20*/	                                                                                                 REST,    N8  ,  NOTE_E5, N8  ,
/*21*/	 NOTE_D5, N8  ,  REST,    N8  ,  NOTE_CS5,N8  ,  REST,    N8  ,  NOTE_A4, N8  ,  REST,    N8  ,  NOTE_A5, N82 ,
#endif

#if		1
// [A]
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*22*/	                                                 REST,    N8  ,  NOTE_A5, N8  ,  NOTE_C6, N8  ,  NOTE_G6, N4  ,
/*23*/	                NOTE_F6,  N4  ,                  NOTE_E6, N4  ,  NOTE_F6, N16 ,  NOTE_E6, N16 ,  NOTE_D6, N8  ,  NOTE_AS5,N82 ,
/*24*/	                                                 REST,    N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_F6, N4  ,
/*25*/	                NOTE_E6,  N4  ,                  NOTE_D6, N4  ,                  NOTE_D6, N8  ,  NOTE_C6, N8  ,  NOTE_G5, N82 ,
/*26*/	                                                 REST,    N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_F6, N4  ,
/*27*/	                NOTE_E6,  N4  ,                  NOTE_D6, N4  ,  NOTE_E6, N16 ,  NOTE_D6, N16 ,  NOTE_C6, N8  ,  NOTE_A5, N0  ,
/*28*/
#endif

#if		1
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*29*/	 REST,    N8  ,  NOTE_F6, N8  ,  REST,    N8  ,  NOTE_E6, N8  ,  REST,    N8  ,  NOTE_G6, N8  ,  REST,    N8  ,  NOTE_F6, N4  ,
/*30*/	                 NOTE_E6, N8  ,  NOTE_D6, N4  ,                  REST,    N8  ,  NOTE_A5, N8  ,  NOTE_C5, N8  ,  NOTE_G6, N4  ,
/*31*/	                 NOTE_F6, N4  ,                  NOTE_E6, N4  ,  NOTE_F6, N16 ,  NOTE_E6, N16 ,  NOTE_D6, N8  ,  NOTE_AS5,N82 ,
/*32*/	                                                                 REST,    N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_F6, N4  ,
/*33*/	                 NOTE_E6, N4  ,                  NOTE_D6, N4  ,  NOTE_E6, N16 ,  NOTE_D6, N16 ,  NOTE_C6, N8  ,  NOTE_G5, N82 ,
/*34*/	                                                 REST,    N8  ,  NOTE_G5, N8  ,  NOTE_AS5,N8  ,  NOTE_F6, N4  ,
/*35*/	 NOTE_E6, N4  ,                  NOTE_D6, N4  ,  NOTE_E6, N16 ,  NOTE_D6,N16  ,  NOTE_C6, N8  ,  NOTE_A5, N81 ,
/*36*/	                                                                                                 NOTE_C6, N8  ,  REST,    N8  ,
/*37*/	 NOTE_F6, N4 ,                   NOTE_E6, N8  ,  NOTE_G6, N82 ,
#endif

#if		0
// [B] 高い周波数の再現が難しい
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*38*/	 NOTE_C7, N2  ,                                                  REST,    N8  ,  NOTE_AS6,N8  ,  REST,    N8  ,  NOTE_A6,  N2 ,
/*39*/	                                                 NOTE_G6, N82 ,
/*40*/	 NOTE_AS6,N4x ,                                  NOTE_A6, N84 ,                                  NOTE_G6, N8  ,  NOTE_A6, N84 ,
/*41*/	                                 NOTE_C6, N8  ,  NOTE_A6, N84 ,                                  REST,    N4  ,
/*42*/	 NOTE_AS6,N2  ,                                                  REST,    N8  ,  NOTE_A6, N8  ,  REST,    N8  ,  NOTE_G6, N2  ,
/*43*/	                                                 NOTE_F6, N84 ,                                  NOTE_G6, N8  ,  REST,    N8  ,
/*44*/	 NOTE_F6, N4x ,                                  NOTE_E6, N8  ,  REST,    N8  ,  NOTE_D6, N8  ,  REST,    N8  ,  NOTE_C6, N2x ,
/*45*/	                                                                 NOTE_AS5,N8  ,  NOTE_F5, N8  ,  NOTE_C6, N82 ,
/*46*/	                                                 REST,    N8  ,  NOTE_D6, N8  ,  NOTE_AS5,N8  ,  NOTE_E6, N2x ,
/*47*/	                                                                 NOTE_F6, N8  ,  NOTE_C6, N8  ,  NOTE_G6, N84 ,
/*48*/	                                 NOTE_D6, N8  ,  NOTE_GS6,N84 ,                                  NOTE_E6, N8  ,  NOTE_A6, N81 ,
/*49*/	                                                                                                 REST,    N8  ,  NOTE_A5, N8  ,
#elif	1
// [B] 1オクターブ下げる
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*38*/	 NOTE_C6, N2  ,                                                  REST,    N8  ,  NOTE_AS5,N8  ,  REST,    N8  ,  NOTE_A5, N2  ,
/*39*/	                                                 NOTE_G5, N82 ,
/*40*/	 NOTE_AS5,N4x ,                                  NOTE_A5, N84 ,                                  NOTE_G5, N8  ,  NOTE_A5, N84 ,
/*41*/	                                 NOTE_C5, N8  ,  NOTE_A5, N84 ,                                  REST,    N4  ,
/*42*/	 NOTE_AS5,N2  ,                                                  REST,    N8  ,  NOTE_A5, N8  ,  REST,    N8  ,  NOTE_G5, N2  ,
/*43*/	                                                 NOTE_F5, N84 ,                                  NOTE_G5, N8  ,  REST,    N8  ,
/*44*/	 NOTE_F5, N4x ,                                  NOTE_E5, N8  ,  REST,    N8  ,  NOTE_D5, N8  ,  REST,    N8  ,  NOTE_C5, N2x ,
/*45*/	                                                                                 NOTE_AS4,N8  ,  NOTE_F4, N8  ,  NOTE_C5, N82 ,
/*46*/	                                                                 REST,    N8  ,  NOTE_D5, N8  ,  NOTE_AS4,N8  ,  NOTE_E5, N2x ,
/*47*/	                                                                                 NOTE_F5, N8  ,  NOTE_C5, N8  ,  NOTE_G5, N84 ,
/*48*/	                                 NOTE_D5, N8  ,  NOTE_GS5,N84 ,                                  NOTE_E5, N8  ,  NOTE_A5, N81 ,
/*49*/	                                                                                                 REST,    N8  ,  NOTE_A5, N8  ,
#endif

#if		1
// [C]
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*50*/	 NOTE_D6, N4  ,                  NOTE_A5, N8  ,  NOTE_E6, N4  ,                  NOTE_A5, N4  ,                  NOTE_F6, N4  ,
/*51*/	                 NOTE_A5, N4  ,                  NOTE_G6, N4  ,                  NOTE_F6, N8  ,  NOTE_E6, N8  ,  NOTE_D6, N8  ,
/*52*/	 NOTE_C6, N4  ,                  NOTE_G5, N8  ,  NOTE_D6, N4  ,                  NOTE_G5, N4  ,                  NOTE_E6, N4  ,
/*53*/	                 NOTE_G5, N4  ,                  NOTE_F6, N4  ,                  NOTE_E6, N8  ,  NOTE_D6, N8  ,  NOTE_C6, N8  ,
/*54*/	 NOTE_AS5,N4  ,                  NOTE_F5, N8  ,  NOTE_C6, N4  ,                  NOTE_F5, N4  ,                  NOTE_D6, N4  ,
/*55*/	                 NOTE_F5, N4  ,                  NOTE_E6, N4  ,                  NOTE_D6, N8  ,  NOTE_C6, N8  ,  NOTE_AS5,N8  ,
/*56*/	 NOTE_A5, N2  ,                                                  NOTE_A5, N8  ,  NOTE_AS5,N8  ,  NOTE_A5, N8  ,  NOTE_G5, N8  ,
/*57*/	 NOTE_F5, N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_AS5,N4  ,                  NOTE_G5, N8  ,  NOTE_C6, N8  ,  REST,    N8  ,
#endif

#if		1
//----	--------------,---------------,---------------,---------------,---------------,---------------,---------------,---------------,
/*58*/	 NOTE_D6, N4  ,                  NOTE_A5, N8  ,  NOTE_E6, N4  ,                  NOTE_A5, N4  ,                  NOTE_F6, N4  ,
/*59*/	                 NOTE_A5, N4  ,                  NOTE_G6, N4  ,                  NOTE_F6, N8  ,  NOTE_E6, N8  ,  NOTE_D6, N8  ,
/*60*/	 NOTE_C6, N4  ,                  NOTE_G5, N8  ,  NOTE_D6, N4  ,                  NOTE_G5, N4  ,                  NOTE_E6, N4  ,
/*61*/	                 NOTE_G5, N4  ,                  NOTE_F6, N4  ,                  NOTE_E6, N8  ,  NOTE_D6, N8  ,  NOTE_C6, N8  ,
/*62*/	 NOTE_AS5,N4  ,                  NOTE_F5, N8  ,  NOTE_C6, N4  ,                  NOTE_F5, N4  ,                  NOTE_D6, N4  ,
/*63*/	                 NOTE_F5, N4  ,                  NOTE_E6, N4  ,                  NOTE_D6, N8  ,  NOTE_C6, N8  ,  NOTE_AS5,N8  ,
/*64*/	 NOTE_A5, N2  ,                                                  NOTE_A5, N8  ,  NOTE_AS5,N8  ,  NOTE_A5, N8  ,  NOTE_G5, N8  ,
/*65*/	 NOTE_F5, N8  ,  NOTE_G5, N8  ,  NOTE_A5, N8  ,  NOTE_AS5,N4  ,                  NOTE_G5, N8  ,  NOTE_C6, N8  ,  NOTE_D6, N8  ,
#endif
