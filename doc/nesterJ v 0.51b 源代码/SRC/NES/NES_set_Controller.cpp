
// data base to set the expand controller automaticaly

ex_controller_type = EX_NONE;
tf_bank = 0;

// Arkanoid Paddle ------------------------------------------------------------

if(crc32() == 0x35893b67 || // Arkanoid (J)
   crc32() == 0x6267fbd1)   // Arkanoid 2 (J)
{
  ex_controller_type = EX_ARKANOID_PADDLE;
}

// Crazy Climber Controller ---------------------------------------------------

if(crc32() == 0xc68363f6)   // Crazy Climber (J)
{
  ex_controller_type = EX_CRAZY_CLIMBER;
}

// Datach Barcode Battler -----------------------------------------------------

if(crc32() == 0x983d8175 || // Datach - Battle Rush - Build Up Robot Tournament (J)
   crc32() == 0x894efdbc || // Datach - Crayon Shin Chan - Ora to Poi Poi (J)
   crc32() == 0x19e81461 || // Datach - Dragon Ball Z - Gekitou Tenkaichi Budou Kai (J)
   crc32() == 0xbe06853f || // Datach - J League Super Top Players (J)
   crc32() == 0x0be0a328 || // Datach - SD Gundam - Gundam Wars (J)
   crc32() == 0x5b457641 || // Datach - Ultraman Club - Supokon Fight! (J)
   crc32() == 0xf51a7f46)   // Datach - Yuu Yuu Hakusho - Bakutou Ankoku Bujutsu Kai (J)
{
  ex_controller_type = EX_DATACH_BARCODE_BATTLER;
}

// Doremikko Keyboard ---------------------------------------------------------

if(fds_id() == 0xa4445245) // Doremikko
{
  ex_controller_type = EX_DOREMIKKO_KEYBOARD;
}

// Exciting Boxing Controller -------------------------------------------------

if(crc32() == 0x786148b6)   // Exciting Boxing (J)
{
  ex_controller_type = EX_EXCITING_BOXING;
}

// Family Basic Keyboard with Data Recorder -----------------------------------

if(                         // Family BASIC (Ver 1.0)
   crc32() == 0xf9def527 || // Family BASIC (Ver 2.0)
   crc32() == 0xde34526e || // Family BASIC (Ver 2.1a)
   crc32() == 0xf050b611 || // Family BASIC (Ver 3)
   crc32() == 0x3aaeed3f    // Family BASIC (Ver 3) (Alt)
                        )   // Play Box Basic
{
    ex_controller_type = EX_FAMILY_KEYBOARD;
}

// Family Trainer -------------------------------------------------------------

if(crc32() == 0x8c8fa83b || // Family Trainer - Athletic World (J)
   crc32() == 0x7e704a14 || // Family Trainer - Jogging Race (J)
   crc32() == 0x2330a5d3)   // Family Trainer - Rairai Kyonshiizu (J)
{
  ex_controller_type = EX_FAMILY_TRAINER_A;
}
if(crc32() == 0xf8da2506 || // Family Trainer - Aerobics Studio (J)
   crc32() == 0xca26a0f1 || // Family Trainer - Dai Undoukai (J)
   crc32() == 0x28068b8c || // Family Trainer - Fuuun Takeshi Jou 2 (J)
   crc32() == 0x10bb8f9a || // Family Trainer - Manhattan Police (J)
   crc32() == 0xad3df455 || // Family Trainer - Meiro Dai Sakusen (J)
   crc32() == 0x8a5b72c0 || // Family Trainer - Running Stadium (J)
   crc32() == 0x59794f2d)   // Family Trainer - Totsugeki Fuuun Takeshi Jou (J)
{
  ex_controller_type = EX_FAMILY_TRAINER_B;
}

// Hyper Shot -----------------------------------------------------------------

if(crc32() == 0xff6621ce || // Hyper Olympic (J)
   crc32() == 0xdb9418e8 || // Hyper Olympic (Tonosama Ban) (J)
   crc32() == 0xac98cd70)   // Hyper Sports (J)
{
  ex_controller_type = EX_HYPER_SHOT;
}

// Mahjong Controller ---------------------------------------------------------

if(crc32() == 0x9fae4d46 || // Ide Yousuke Meijin no Jissen Mahjong (J)
   crc32() == 0x7b44fb2a)   // Ide Yousuke Meijin no Jissen Mahjong 2 (J)
{
  ex_controller_type = EX_MAHJONG;
}

// Oeka Kids Tablet -----------------------------------------------------------

if(crc32() == 0xc3c0811d || // Oeka Kids - Anpanman no Hiragana Daisuki (J)
   crc32() == 0x9d048ea4)   // Oeka Kids - Anpanman to Oekaki Shiyou!! (J)
{
  ex_controller_type = EX_OEKAKIDS_TABLET;
}

// Optical Gun (Zapper) -------------------------------------------------------

if(crc32() == 0xfbfc6a6c || // Adventures of Bayou Billy, The (E)
   crc32() == 0xcb275051 || // Adventures of Bayou Billy, The (U)
   crc32() == 0xfb69c131 || // Baby Boomer (Unl) (U)
   crc32() == 0xf2641ad0 || // Barker Bill's Trick Shooting (U)
   crc32() == 0xbc1dce96 || // Chiller (Unl) (U)
   crc32() == 0x90ca616d || // Duck Hunt (JUE)
   crc32() == 0x59e3343f || // Freedom Force (U)
   crc32() == 0x242a270c || // Gotcha! (U)
   crc32() == 0x7b5bd2de || // Gumshoe (UE)
   crc32() == 0x255b129c || // Gun Sight (J)
   crc32() == 0x8963ae6e || // Hogan's Alley (JU)
   crc32() == 0x51d2112f || // Laser Invasion (U)
   crc32() == 0x0a866c94 || // Lone Ranger, The (U)
   crc32() == 0xe4c04eea || // Mad City (J)
   crc32() == 0x9eef47aa || // Mechanized Attack (U)
   crc32() == 0xc2db7551 || // Shooting Range (U)
   crc32() == 0x163e86c0 || // To The Earth (U)
   crc32() == 0x389960db)   // Wild Gunman (JUE)
{
  ex_controller_type = EX_OPTICAL_GUN;
}

// Pokkun Moguraa Controller --------------------------------------------------

if(crc32() == 0x3993b4eb)   // Pokkun Moguraa (J)
{
  ex_controller_type = EX_POKKUN_MOGURAA;
}

// Power Pad ------------------------------------------------------------------

if(crc32() == 0xbc5f6c94)   // Athletic World (U)
{
  ex_controller_type = EX_POWER_PAD_A;
}
if(crc32() == 0xd836a90b || // Dance Aerobics (U)
   crc32() == 0x96c4ce38 || // Short Order - Eggsplode (U)
   crc32() == 0x987dcda3)   // Street Cop (U)
{
  ex_controller_type = EX_POWER_PAD_B;
}

// Space Shadow Gun (Hyper Shot) ----------------------------------------------

if(crc32() == 0x0cd00488)   // Space Shadow (J)
{
  ex_controller_type = EX_SPACE_SHADOW_GUN;
}

// Top Rider Controller -------------------------------------------------------

if(crc32() == 0x20d22251)   // Top Rider (J)
{
  //ex_controller_type = EX_TOP_RIDER;
}

// Turbo File -----------------------------------------------------------------

if(crc32() == 0xe792de94 || // Best Play - Pro Yakyuu (New) (J)
   crc32() == 0xf79d684a)   // Best Play - Pro Yakyuu (Old) (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 0;
}
if(crc32() == 0xc2ef3422)   // Best Play - Pro Yakyuu 2 (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 1;
}
if(crc32() == 0x974e8840)   // Best Play - Pro Yakyuu '90 (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 2;
}
if(crc32() == 0xb8747abf)   // Best Play - Pro Yakyuu Special (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 3;
}
if(crc32() == 0x9fa1c11f)   // Castle Excellent (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 4;
}
if(crc32() == 0x0b0d4d1b)   // Derby Stallion - Zenkoku Ban (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 5;
}
if(crc32() == 0x728c3d98)   // Downtown - Nekketsu Monogatari (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 6;
}
if(crc32() == 0xd68a6f33)   // Dungeon Kid (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 7;
}
if(crc32() == 0x3a51eb04)   // Fleet Commander (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 8;
}
if(crc32() == 0x7c46998b)   // Haja no Fuuin (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 9;
}
if(crc32() == 0x7e5d2f1a)   // Itadaki Street - Watashi no Mise ni Yottette (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 10;
}
if(crc32() == 0xcee5857b)   // Ninjara Hoi! (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 11;
}
if(crc32() == 0x50ec5e8b || // Wizardry - Legacy of Llylgamyn (J)
   crc32() == 0x343e9146 || // Wizardry - Proving Grounds of the Mad Overlord (J)
   crc32() == 0x33d07e45)   // Wizardry - The Knight of Diamonds (J)
{
  ex_controller_type = EX_TURBO_FILE;
  tf_bank = 12;
}

// VS Unisystem Zapper --------------------------------------------------------

if(crc32() == 0xed588f00 || // VS Duck Hunt
   crc32() == 0x17ae56be || // VS Freedom Force.nes
   crc32() == 0xff5135a3)   // VS Hogan's Alley
{
  ex_controller_type = EX_VS_ZAPPER;
}

