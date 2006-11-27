
if(                         // Gradius AC 2000 (Hack)
   crc32() == 0xaeac7339 || //   GraAck2k                               
   crc32() == 0xdd6c5ec4 || //   GraAck2k                       + Konami
   crc32() == 0xc077477f || //   GraAck2k            + GraAc2kL         
   crc32() == 0xb3b76a82 || //   GraAck2k            + GraAc2kL + Konami
   crc32() == 0x49aadad1 || //   GraAck2k + GraSound                    
   crc32() == 0x3a6af72c || //   GraAck2k + GraSound            + Konami
   crc32() == 0x2771ee97 || //   GraAck2k + GraSound + GraAc2kL         
   crc32() == 0x54b1c36a)   //   GraAck2k + GraSound + GraAc2kL + Konami
{
  ppu->sprite0_hit_flag = 1;
}

