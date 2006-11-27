
// Arkanoid Paddle ------------------------------------------------------------

uint8 NES::ReadReg4016_ARKANOID_PADDLE()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_ARKANOID_PADDLE)
  {
    retval = arkanoid_byte;
  }
  return retval;
}

uint8 NES::ReadReg4017_ARKANOID_PADDLE()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_ARKANOID_PADDLE)
  {
    retval = (arkanoid_bits & 0x01) << 1;
    arkanoid_bits >>= 1;
  }
  return retval;
}

void NES::WriteReg4016_strobe_ARKANOID_PADDLE()
{
  if(ex_controller_type == EX_ARKANOID_PADDLE)
  {
    if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) pad1_bits |= 0x04;
  }
}

void NES::WriteReg4016_ARKANOID_PADDLE(uint8 data)
{
  arkanoid_byte = 0x00;
  arkanoid_bits = 0x00;

  if(ex_controller_type == EX_ARKANOID_PADDLE)
  {
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) arkanoid_byte = 0x02;
    try
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(main_window_handle, &pt);
      if(NESTER_settings.nes.graphics.osd.double_size)
      {
        pt.x >>= 1;
      }
      if(pt.x > 176) pt.x = 176;
      if(pt.x < 32) pt.x = 32;
      uint8 px = 0xFF - (uint8)(0x52+172*(pt.x-32)/144);
      arkanoid_bits |= (px & 0x01) << 7;
      arkanoid_bits |= (px & 0x02) << 5;
      arkanoid_bits |= (px & 0x04) << 3;
      arkanoid_bits |= (px & 0x08) << 1;
      arkanoid_bits |= (px & 0x10) >> 1;
      arkanoid_bits |= (px & 0x20) >> 3;
      arkanoid_bits |= (px & 0x40) >> 5;
      arkanoid_bits |= (px & 0x80) >> 7;
    } catch(...) { }
  }
}

// Crazy Climber --------------------------------------------------------------

void NES::WriteReg4016_strobe_CRAZY_CLIMBER()
{
  if(ex_controller_type == EX_CRAZY_CLIMBER)
  {
    pad1_bits = pad1_bits & 0x0F;
    pad2_bits = pad2_bits & 0x0F;
    if(GetAsyncKeyState('D') & 0x8000) pad1_bits |= 0x10;
    if(GetAsyncKeyState('A') & 0x8000) pad1_bits |= 0x20;
    if(GetAsyncKeyState('W') & 0x8000) pad1_bits |= 0x40;
    if(GetAsyncKeyState('S') & 0x8000) pad1_bits |= 0x80;
    if(GetAsyncKeyState('J') & 0x8000) pad2_bits |= 0x10;
    if(GetAsyncKeyState('G') & 0x8000) pad2_bits |= 0x20;
    if(GetAsyncKeyState('Y') & 0x8000) pad2_bits |= 0x40;
    if(GetAsyncKeyState('H') & 0x8000) pad2_bits |= 0x80;
  }
}

// Doremikko Keyboard ---------------------------------------------------------

uint8 NES::ReadReg4017_DOREMIKKO_KEYBOARD()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_DOREMIKKO_KEYBOARD)
  {
    switch(doremi_scan)
    {
      case 1:
        {
          if(GetAsyncKeyState('Z') & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('S') & 0x8000) retval |= 0x20;
        }
        break;

      case 2:
        {
          if(GetAsyncKeyState('X') & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState('D') & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('C') & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('V') & 0x8000) retval |= 0x08;
          if(GetAsyncKeyState('G') & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('B') & 0x8000) retval |= 0x20;
        }
        break;

      case 3:
        {
          if(GetAsyncKeyState('H') & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState('N') & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('J') & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('M') & 0x8000) retval |= 0x08;
          if(GetAsyncKeyState(','+0x90) & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('L') & 0x8000) retval |= 0x20;
        }
        break;

      case 4:
        {
          if(GetAsyncKeyState('.'+0x90) & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState(';'+0x80) & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('/'+0x90) & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('Q') & 0x8000) retval |= 0x08;
          if(GetAsyncKeyState('2') & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('W') & 0x8000) retval |= 0x20;
        }
        break;

      case 5:
        {
          if(GetAsyncKeyState('3') & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState('E') & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('4') & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('R') & 0x8000) retval |= 0x08;
          if(GetAsyncKeyState('T') & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('6') & 0x8000) retval |= 0x20;
        }
        break;

      case 6:
        {
          if(GetAsyncKeyState('Y') & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState('7') & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('U') & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('I') & 0x8000) retval |= 0x08;
          if(GetAsyncKeyState('9') & 0x8000) retval |= 0x10;
          if(GetAsyncKeyState('O') & 0x8000) retval |= 0x20;
        }
        break;

      case 7:
        {
          if(GetAsyncKeyState('0') & 0x8000) retval |= 0x01;
          if(GetAsyncKeyState('P') & 0x8000) retval |= 0x02;
          if(GetAsyncKeyState('-'+0x90) & 0x8000) retval |= 0x04;
          if(GetAsyncKeyState('@'+0x80) & 0x8000) retval |= 0x08;
        }
        break;
    }
    if(doremi_out)
    {
      retval = (retval & 0xF0) >> 3;
    }
    else
    {
      retval = (retval & 0x0F) << 1;
    }
    //retval = (0xFF - retval) & 0x1e;
    doremi_out = 1 - doremi_out;
  }
  return retval;
}

void NES::WriteReg4016_DOREMIKKO_KEYBOARD(uint8 data)
{
  if (ex_controller_type == EX_DOREMIKKO_KEYBOARD)
  {
    if((data & 0x02) && !(doremi_reg & 0x02))
    {
      doremi_scan = 0;
      doremi_out = 0;
    }
    if((data & 0x01) && !(doremi_reg & 0x01))
    {
      doremi_scan++;
      doremi_out = 0;
    }
    doremi_reg = data;
  }
}

// Exciting Boxing ------------------------------------------------------------

uint8 NES::ReadReg4017_EXCITING_BOXING()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_EXCITING_BOXING)
  {
    retval = excitingboxing_byte;
  }
  return retval;
}

void NES::WriteReg4016_EXCITING_BOXING(uint8 data)
{
  excitingboxing_byte = 0x00;

  if(ex_controller_type == EX_EXCITING_BOXING)
  {
    if(data & 0x02)
    {
      if(!(GetAsyncKeyState('D') & 0x8000)) excitingboxing_byte |= 0x10;
      if(!(GetAsyncKeyState('E') & 0x8000)) excitingboxing_byte |= 0x08;
      if(!(GetAsyncKeyState('S') & 0x8000)) excitingboxing_byte |= 0x04;
      if(!(GetAsyncKeyState('W') & 0x8000)) excitingboxing_byte |= 0x02;
    }
    else
    {
      if(!(GetAsyncKeyState('R') & 0x8000)) excitingboxing_byte |= 0x10;
      if(!(GetAsyncKeyState('F') & 0x8000)) excitingboxing_byte |= 0x08;
      if(!(GetAsyncKeyState('A') & 0x8000)) excitingboxing_byte |= 0x04;
      if(!(GetAsyncKeyState('Q') & 0x8000)) excitingboxing_byte |= 0x02;
    }
  }
}

// Family Basic Keyboard (thanx rinao) and Data Recorder ----------------------

uint8 NES::ReadReg4016_FAMILY_KEYBOARD()
{
  uint8 retval = 0x00;

  if (ex_controller_type == EX_FAMILY_KEYBOARD)
  {
    if(tape_status == 1 && tape_in)
    {
      retval = 0x02;
    }
  }
  return retval;
}

uint8 NES::ReadReg4017_FAMILY_KEYBOARD()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_FAMILY_KEYBOARD)
  {
    retval = 0xFF;

    if(GetAsyncKeyState(VK_NEXT) & 0x8000) kb_graph = 1;
    if(GetAsyncKeyState(VK_PRIOR) & 0x8000) kb_graph = 0;

    if(kb_out)
    {
      switch(kb_scan)
      {
        case 1:
          {
            if(GetAsyncKeyState(VK_F8) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState(VK_RETURN) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('[' + 0x80) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(']' + 0x80) & 0x8000) retval &= 0xEF;
          }
          break;

        case 2:
          {
            if(GetAsyncKeyState(VK_F7) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('@' + 0x80) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState(':' + 0x80) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(';' + 0x80) & 0x8000) retval &= 0xEF;
          }
          break;

        case 3:
          {
            if(GetAsyncKeyState(VK_F6) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('O') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('L') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('K') & 0x8000) retval &= 0xEF;
          }
          break;

        case 4:
          {
            if(GetAsyncKeyState(VK_F5) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('I') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('U') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('J') & 0x8000) retval &= 0xEF;
          }
          break;

        case 5:
          {
            if(GetAsyncKeyState(VK_F4) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('Y') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('G') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('H') & 0x8000) retval &= 0xEF;
          }
          break;

        case 6:
          {
            if(GetAsyncKeyState(VK_F3) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('T') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('R') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('D') & 0x8000) retval &= 0xEF;
          }
          break;

        case 7:
          {
            if(GetAsyncKeyState(VK_F2) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('W') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('S') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('A') & 0x8000) retval &= 0xEF;
          }
          break;

        case 8:
          {
            if(GetAsyncKeyState(VK_F1) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState(VK_ESCAPE) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('Q') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(VK_CONTROL)) retval &= 0xEF;
          }
          break;

        case 9:
          {
            if(GetAsyncKeyState(VK_HOME) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState(VK_UP) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState(VK_RIGHT) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(VK_LEFT) & 0x8000) retval &= 0xEF;
          }
          break;
      }
    }
    else
    {
      switch(kb_scan)
      {
        case 1:
          {
            if(GetAsyncKeyState(VK_F12) & 0x8000) retval &= 0xFD;
            //if(GetAsyncKeyState(VK_SHIFT)) retval &= 0xFB;  //shift(right)
            if(GetAsyncKeyState('\\' + 0x80) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(VK_END) & 0x8000) retval &= 0xEF;
          }
          break;

        case 2:
          {
            if(GetAsyncKeyState(226) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('/' + 0x90) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('-' + 0x90) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('^' + 0x80) & 0x8000) retval &= 0xEF;
          }
          break;

        case 3:
          {
            if(GetAsyncKeyState('.' + 0x90) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState(',' + 0x90) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('P') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('0') & 0x8000) retval &= 0xEF;
          }
          break;

        case 4:
          {
            if(GetAsyncKeyState('M') & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('N') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('9') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('8') & 0x8000) retval &= 0xEF;
          }
          break;

        case 5:
          {
            if(GetAsyncKeyState('B') & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('V') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('7') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('6') & 0x8000) retval &= 0xEF;
          }
          break;

        case 6:
          {
            if(GetAsyncKeyState('F') & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('C') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('5') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('4') & 0x8000) retval &= 0xEF;
          }
          break;

        case 7:
          {
            if(GetAsyncKeyState('X') & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState('Z') & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState('E') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('3') & 0x8000) retval &= 0xEF;
          }
          break;

        case 8:
          {
            if(GetAsyncKeyState(VK_SHIFT)) retval &= 0xFD;
            //if(GetAsyncKeyState(VK_MENU)) retval &= 0xFB;
            if(kb_graph) retval &= 0xFB;
            if(GetAsyncKeyState('1') & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState('2') & 0x8000) retval &= 0xEF;
          }
          break;

        case 9:
          {
            if(GetAsyncKeyState(VK_DOWN) & 0x8000) retval &= 0xFD;
            if(GetAsyncKeyState(VK_SPACE) & 0x8000) retval &= 0xFB;
            if(GetAsyncKeyState(VK_DELETE) & 0x8000) retval &= 0xF7;
            if(GetAsyncKeyState(VK_INSERT) & 0x8000) retval &= 0xEF;
          }
          break;
      }
    }
  }
  return retval;
}

void NES::WriteReg4016_FAMILY_KEYBOARD(uint8 data)
{
  if (ex_controller_type == EX_FAMILY_KEYBOARD)
  {
    if (data == 0x05)
    {
      kb_out = 0;
      kb_scan = 0;
    }
    else if (data  == 0x04)
    {
      kb_scan++;
      if (kb_scan > 9) kb_scan = 0;
      kb_out = 1 - kb_out;
    }
    else if (data == 0x06)
    {
      kb_out = 1 - kb_out;
    }
    if (tape_status == 2)
    {
      tape_out = data & 0x02;
    }
  }
}

void NES::RotateTape()
{
  if(tape_status == 1)
  {
    if(tape_bit == 0x80)
    {
      tape_bit = 0x01;
      int tmp_data;
      if((tmp_data = fgetc(ftape)) == EOF)
      {
        tape_status = 0;
        tape_in = 0;
      }
      else
      {
        tape_data = tmp_data;
        tape_in = tape_data & tape_bit;
      }
    }
    else
    {
      tape_bit <<= 1;
      tape_in = tape_data & tape_bit;
    }
  }
  else if(tape_status == 2)
  {
    if(tape_out)
    {
      tape_data |= tape_bit;
    }
    else
    {
      tape_data &= 0xFF - tape_bit;
    }
    if(tape_bit == 0x80)
    {
      fputc(tape_data, ftape);
      tape_bit = 0x01;
    }
    else
    {
      tape_bit <<= 1;
    }
  }
}

// Family Trainer -------------------------------------------------------------

uint8 NES::ReadReg4017_FAMILY_TRAINER()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_FAMILY_TRAINER_A ||
     ex_controller_type == EX_FAMILY_TRAINER_B)
  {
    retval = familytrainer_byte;
  }
  return retval;
}

void NES::WriteReg4016_strobe_FAMILY_TRAINER()
{
  if(ex_controller_type == EX_FAMILY_TRAINER_A ||
     ex_controller_type == EX_FAMILY_TRAINER_B)
  {
    if(GetAsyncKeyState('T') & 0x8000) pad1_bits |= 0x08;
    if(GetAsyncKeyState('G') & 0x8000) pad1_bits |= 0x04;
  }
}

void NES::WriteReg4016_FAMILY_TRAINER(uint8 data)
{
  familytrainer_byte = 0x00;

  if(ex_controller_type == EX_FAMILY_TRAINER_A)
  {
    if(!(data & 0x04))
    {
      if(!(GetAsyncKeyState('R') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('E') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('W') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('Q') & 0x8000)) familytrainer_byte |= 0x02;
    }
    if(!(data & 0x02))
    {
      if(!(GetAsyncKeyState('F') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('D') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('S') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('A') & 0x8000)) familytrainer_byte |= 0x02;
    }
    if(!(data & 0x01))
    {
      if(!(GetAsyncKeyState('V') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('C') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('X') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('Z') & 0x8000)) familytrainer_byte |= 0x02;
    }
  }
  if(ex_controller_type == EX_FAMILY_TRAINER_B)
  {
    if(!(data & 0x04))
    {
      if(!(GetAsyncKeyState('Q') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('W') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('E') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('R') & 0x8000)) familytrainer_byte |= 0x02;
    }
    if(!(data & 0x02))
    {
      if(!(GetAsyncKeyState('A') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('S') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('D') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('F') & 0x8000)) familytrainer_byte |= 0x02;
    }
    if(!(data & 0x01))
    {
      if(!(GetAsyncKeyState('Z') & 0x8000)) familytrainer_byte |= 0x10;
      if(!(GetAsyncKeyState('X') & 0x8000)) familytrainer_byte |= 0x08;
      if(!(GetAsyncKeyState('C') & 0x8000)) familytrainer_byte |= 0x04;
      if(!(GetAsyncKeyState('V') & 0x8000)) familytrainer_byte |= 0x02;
    }
  }
}

// Hyper Shot -----------------------------------------------------------------

uint8 NES::ReadReg4017_HYPER_SHOT()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_HYPER_SHOT)
  {
    retval = hypershot_byte;
  }
  return retval;
}

void NES::WriteReg4016_strobe_HYPER_SHOT()
{
  if(ex_controller_type == EX_HYPER_SHOT)
  {
    hypershot_byte = 0x00;
    if(pad1_bits & 0x01) hypershot_byte |= 0x02;
    if(pad1_bits & 0x02) hypershot_byte |= 0x04;
    if(pad2_bits & 0x01) hypershot_byte |= 0x08;
    if(pad2_bits & 0x02) hypershot_byte |= 0x10;
  }
}

// Mahjong Controller ---------------------------------------------------------

uint8 NES::ReadReg4017_MAHJONG()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_MAHJONG)
  {
    retval = (mahjong_bits & 0x01) << 1;
    mahjong_bits >>= 1;
  }
  return retval;
}

void NES::WriteReg4016_MAHJONG(uint8 data)
{
  mahjong_bits = 0x00;

  if(ex_controller_type == EX_MAHJONG)
  {
    if((data & 0x06) == 0x02)
    {
      if(GetAsyncKeyState('J') & 0x8000) mahjong_bits |= 0x04;
      if(GetAsyncKeyState('H') & 0x8000) mahjong_bits |= 0x08;
      if(GetAsyncKeyState('G') & 0x8000) mahjong_bits |= 0x10;
      if(GetAsyncKeyState('F') & 0x8000) mahjong_bits |= 0x20;
      if(GetAsyncKeyState('D') & 0x8000) mahjong_bits |= 0x40;
      if(GetAsyncKeyState('S') & 0x8000) mahjong_bits |= 0x80;
    }
    else if((data & 0x06) == 0x04)
    {
      if(GetAsyncKeyState('A') & 0x8000) mahjong_bits |= 0x01;
      if(GetAsyncKeyState('U') & 0x8000) mahjong_bits |= 0x02;
      if(GetAsyncKeyState('Y') & 0x8000) mahjong_bits |= 0x04;
      if(GetAsyncKeyState('T') & 0x8000) mahjong_bits |= 0x08;
      if(GetAsyncKeyState('R') & 0x8000) mahjong_bits |= 0x10;
      if(GetAsyncKeyState('E') & 0x8000) mahjong_bits |= 0x20;
      if(GetAsyncKeyState('W') & 0x8000) mahjong_bits |= 0x40;
      if(GetAsyncKeyState('Q') & 0x8000) mahjong_bits |= 0x80;
    }
    else if((data & 0x06) == 0x06)
    {
      if(GetAsyncKeyState('M') & 0x8000) mahjong_bits |= 0x02;
      if(GetAsyncKeyState('N') & 0x8000) mahjong_bits |= 0x04;
      if(GetAsyncKeyState('B') & 0x8000) mahjong_bits |= 0x08;
      if(GetAsyncKeyState('V') & 0x8000) mahjong_bits |= 0x10;
      if(GetAsyncKeyState('C') & 0x8000) mahjong_bits |= 0x20;
      if(GetAsyncKeyState('Z') & 0x8000) mahjong_bits |= 0x40;
      if(GetAsyncKeyState('X') & 0x8000) mahjong_bits |= 0x80;
    }
  }
}

// OekaKids Tablet ------------------------------------------------------------

uint8 NES::ReadReg4017_OEKAKIDS_TABLET()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_OEKAKIDS_TABLET)
  {
    retval = tablet_byte;
  }
  return retval;
}

void NES::WriteReg4016_OEKAKIDS_TABLET(uint8 data)
{
  if(ex_controller_type == EX_OEKAKIDS_TABLET)
  {
    if(!(data & 0x01))
    {
      tablet_byte = 0;
      tablet_data = 0;
      if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0001;
      try
      {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(main_window_handle, &pt);
        if(NESTER_settings.nes.graphics.osd.double_size)
        {
          pt.x >>= 1;
          pt.y >>= 1;
        }
        if(!NESTER_settings.nes.graphics.show_all_scanlines)
        {
          pt.y += 8;
        }
        if(pt.y > 48)
        {
          if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0001;
          tablet_data |= 0x0002;
        }
        else
        {
          if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) tablet_data |= 0x0003;
        }
        pt.x = (int32)((240*pt.x)/256) +  8; if(pt.x < 0) pt.x = 0;
        pt.y = (int32)((256*pt.y)/240) - 12; if(pt.y < 0) pt.y = 0;
        tablet_data = tablet_data | (pt.x << 10) | (pt.y << 2);
      } catch(...) { }
    }
    else
    {
      if(!(tablet_pre_flag & 0x02) && (data & 0x02))
      {
        tablet_data <<= 1;
      }
      if(!(data & 0x02))
      {
        tablet_byte = 0x04;
      }
      else
      {
        if(tablet_data & 0x40000)
        {
          tablet_byte = 0x00;
        }
        else
        {
          tablet_byte = 0x08;
        }
      }
    }
    tablet_pre_flag = data;
  }
}

// Optical Gun / Zapper -------------------------------------------------------

uint8 NES::ReadReg4017_OPTICAL_GUN()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_OPTICAL_GUN)
  {
    retval = 0x08;
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) retval |= 0x10;
    try
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(main_window_handle, &pt);
      if(NESTER_settings.nes.graphics.osd.double_size)
      {
        pt.x >>= 1;
        pt.y >>= 1;
      }
      if(!NESTER_settings.nes.graphics.show_all_scanlines)
      {
        pt.y += 8;
      }

      if(0 < pt.x && pt.x < 255 && 0 < pt.y && pt.y < 239)
      {
        uint8 c = ppu->GetPointColor(pt.x, pt.y) & 0x3F;
        if(crc32() == 0xbc1dce96) // Chiller (#11)
        {
          if(NES_RGB_pal[c][0] > 0 && NES_RGB_pal[c][1] == 0 && NES_RGB_pal[c][2] > 0)
          {
            retval &= 0xF7;
          }
        }
        else
        {
          if(c == 32 || c == 48)
          //if(NES_RGB_pal[c][0] == 0xFF && NES_RGB_pal[c][1] == 0xFF && NES_RGB_pal[c][2] == 0xFF)
          {
            retval &= 0xF7;
          }
        }
      }
    } catch(...) { }
  }
  return retval;
}

// Pokkun Moguraa -------------------------------------------------------------

uint8 NES::ReadReg4017_POKKUN_MOGURAA()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_POKKUN_MOGURAA)
  {
    retval = pokkunmoguraa_byte;
  }
  return retval;
}

void NES::WriteReg4016_strobe_POKKUN_MOGURAA()
{
  if(ex_controller_type == EX_POKKUN_MOGURAA)
  {
    if(GetAsyncKeyState('T') & 0x8000) pad1_bits |= 0x08;
    if(GetAsyncKeyState('G') & 0x8000) pad1_bits |= 0x04;
  }
}

void NES::WriteReg4016_POKKUN_MOGURAA(uint8 data)
{
  pokkunmoguraa_byte = 0x00;

  if(ex_controller_type == EX_POKKUN_MOGURAA)
  {
    if(!(data & 0x04))
    {
      if(!(GetAsyncKeyState('R') & 0x8000)) pokkunmoguraa_byte |= 0x10;
      if(!(GetAsyncKeyState('E') & 0x8000)) pokkunmoguraa_byte |= 0x08;
      if(!(GetAsyncKeyState('W') & 0x8000)) pokkunmoguraa_byte |= 0x04;
      if(!(GetAsyncKeyState('Q') & 0x8000)) pokkunmoguraa_byte |= 0x02;
    }
    if(!(data & 0x02))
    {
      if(!(GetAsyncKeyState('F') & 0x8000)) pokkunmoguraa_byte |= 0x10;
      if(!(GetAsyncKeyState('D') & 0x8000)) pokkunmoguraa_byte |= 0x08;
      if(!(GetAsyncKeyState('S') & 0x8000)) pokkunmoguraa_byte |= 0x04;
      if(!(GetAsyncKeyState('A') & 0x8000)) pokkunmoguraa_byte |= 0x02;
    }
    if(!(data & 0x01))
    {
      if(!(GetAsyncKeyState('V') & 0x8000)) pokkunmoguraa_byte |= 0x10;
      if(!(GetAsyncKeyState('C') & 0x8000)) pokkunmoguraa_byte |= 0x08;
      if(!(GetAsyncKeyState('X') & 0x8000)) pokkunmoguraa_byte |= 0x04;
      if(!(GetAsyncKeyState('Z') & 0x8000)) pokkunmoguraa_byte |= 0x02;
    }
  }
}

// Power Pad ------------------------------------------------------------------

uint8 NES::ReadReg4017_POWER_PAD()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_POWER_PAD_A ||
     ex_controller_type == EX_POWER_PAD_B)
  {
    retval = 0x00;
    retval |= (powerpad_bits1 & 0x01) << 3;
    retval |= (powerpad_bits2 & 0x01) << 4;
    powerpad_bits1 >>= 1;
    powerpad_bits2 >>= 1;
  }
  return retval;
}

void NES::WriteReg4016_strobe_POWER_PAD()
{
  powerpad_bits1 = powerpad_bits2 = 0x00;

  if(ex_controller_type == EX_POWER_PAD_A)
  {
    if(GetAsyncKeyState('R') & 0x8000) powerpad_bits1 |= 0x02;
    if(GetAsyncKeyState('E') & 0x8000) powerpad_bits1 |= 0x01;
    if(GetAsyncKeyState('W') & 0x8000) powerpad_bits2 |= 0x02;
    if(GetAsyncKeyState('Q') & 0x8000) powerpad_bits2 |= 0x01;
    if(GetAsyncKeyState('F') & 0x8000) powerpad_bits1 |= 0x04;
    if(GetAsyncKeyState('D') & 0x8000) powerpad_bits1 |= 0x10;
    if(GetAsyncKeyState('S') & 0x8000) powerpad_bits1 |= 0x80;
    if(GetAsyncKeyState('A') & 0x8000) powerpad_bits2 |= 0x08;
    if(GetAsyncKeyState('V') & 0x8000) powerpad_bits1 |= 0x08;
    if(GetAsyncKeyState('C') & 0x8000) powerpad_bits1 |= 0x20;
    if(GetAsyncKeyState('X') & 0x8000) powerpad_bits1 |= 0x40;
    if(GetAsyncKeyState('Z') & 0x8000) powerpad_bits2 |= 0x04;
    if(GetAsyncKeyState('T') & 0x8000) pad1_bits |= 0x08;
    if(GetAsyncKeyState('G') & 0x8000) pad1_bits |= 0x04;
  }
  if(ex_controller_type == EX_POWER_PAD_B)
  {
    if(GetAsyncKeyState('Q') & 0x8000) powerpad_bits1 |= 0x02;
    if(GetAsyncKeyState('W') & 0x8000) powerpad_bits1 |= 0x01;
    if(GetAsyncKeyState('E') & 0x8000) powerpad_bits2 |= 0x02;
    if(GetAsyncKeyState('R') & 0x8000) powerpad_bits2 |= 0x01;
    if(GetAsyncKeyState('A') & 0x8000) powerpad_bits1 |= 0x04;
    if(GetAsyncKeyState('S') & 0x8000) powerpad_bits1 |= 0x10;
    if(GetAsyncKeyState('D') & 0x8000) powerpad_bits1 |= 0x80;
    if(GetAsyncKeyState('F') & 0x8000) powerpad_bits2 |= 0x08;
    if(GetAsyncKeyState('Z') & 0x8000) powerpad_bits1 |= 0x08;
    if(GetAsyncKeyState('X') & 0x8000) powerpad_bits1 |= 0x20;
    if(GetAsyncKeyState('C') & 0x8000) powerpad_bits1 |= 0x40;
    if(GetAsyncKeyState('V') & 0x8000) powerpad_bits2 |= 0x04;
    if(GetAsyncKeyState('T') & 0x8000) pad1_bits |= 0x08;
    if(GetAsyncKeyState('G') & 0x8000) pad1_bits |= 0x04;
  }
}

// Space Shadow Gun -----------------------------------------------------------

uint8 NES::ReadReg4016_SPACE_SHADOW_GUN()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_SPACE_SHADOW_GUN)
  {
    retval = (spaceshadow_bits & 0x01) << 1;
    spaceshadow_bits >>= 1;
  }
  return retval;
}

uint8 NES::ReadReg4017_SPACE_SHADOW_GUN()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_SPACE_SHADOW_GUN)
  {
    retval = 0x08;
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) retval |= 0x10;
    try
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(main_window_handle, &pt);
      if(NESTER_settings.nes.graphics.osd.double_size)
      {
        pt.x >>= 1;
        pt.y >>= 1;
      }
      if(!NESTER_settings.nes.graphics.show_all_scanlines)
      {
        pt.y += 8;
      }

      if(0 < pt.x && pt.x < 255 && 0 < pt.y && pt.y < 239)
      {
        uint8 c = ppu->GetPointColor(pt.x, pt.y) & 0x3F;
        if(NES_RGB_pal[c][0] == 0xFF && NES_RGB_pal[c][1] == 0xFF && NES_RGB_pal[c][2] == 0xFF)
        {
          retval &= 0xF7;
        }
      }
    } catch(...) { }
  }
  return retval;
}

void NES::WriteReg4016_strobe_SPACE_SHADOW_GUN()
{
  spaceshadow_bits = 0x00;

  if(ex_controller_type == EX_SPACE_SHADOW_GUN)
  {
    spaceshadow_bits = pad1_bits & 0xFC;
    spaceshadow_bits |= (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 0x02 : 0x00;
  }
}

// Turbo File -----------------------------------------------------------------

uint8 NES::ReadReg4017_TURBO_FILE()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_TURBO_FILE)
  {
    retval = tf_byte;
  }
  return retval;
}

void NES::WriteReg4016_TURBO_FILE(uint8 data)
{
  tf_byte = 0x00;

  if(ex_controller_type == EX_TURBO_FILE)
  {
    if(!(data & 0x02))
    {
      tf_pointer = 0x00;
      tf_bit = 0x01;
    }
    if(data & 0x04)
    {
      tf_data[tf_bank*0x2000 + tf_pointer] &= 0xFF - tf_bit;
      tf_data[tf_bank*0x2000 + tf_pointer] |= tf_bit * (data & 0x01);
       tf_write = 1;
    }
    if((tf_pre_flag & 0x04) && !(data & 0x04))
    {
      if(tf_bit == 0x80)
      {
        tf_pointer = (tf_pointer + 1) & 0x1FFF;
        tf_bit = 0x01;
      }
      else
      {
        tf_bit <<= 1;
      }
    }
    if(tf_data[tf_bank*0x2000 + tf_pointer] & tf_bit)
    {
      tf_byte = 0x04;
    }
    tf_pre_flag = data;
  }
}

// VS Unisystem Zapper --------------------------------------------------------

uint8 NES::ReadReg4016_VS_ZAPPER()
{
  uint8 retval = 0x00;

  if(ex_controller_type == EX_VS_ZAPPER)
  {
    if(vszapper_count == 4)
    {
      retval = 0x01;
    }
    vszapper_count++;
  }
  return retval;
}

void NES::WriteReg4016_strobe_VS_ZAPPER()
{
  if(ex_controller_type == EX_VS_ZAPPER)
  {
    if(GetAsyncKeyState(VK_LBUTTON) & 0x8000)
    {
      pad1_bits |= 0x80;
    }
    else
    {
      pad1_bits &= 0x7E;
    }
    try
    {
      POINT pt;
      GetCursorPos(&pt);
      ScreenToClient(main_window_handle, &pt);
      if(NESTER_settings.nes.graphics.osd.double_size)
      {
        pt.x >>= 1;
        pt.y >>= 1;
      }
      if(!NESTER_settings.nes.graphics.show_all_scanlines)
      {
        pt.y += 8;
      }
      if(0 < pt.x && pt.x < 255 && 0 < pt.y && pt.y < 239)
      {
        uint8 c = ppu->GetPointColor(pt.x, pt.y) & 0x3F;
        if(NES_RGB_pal[c][0] == 0xFF && NES_RGB_pal[c][1] == 0xFF && NES_RGB_pal[c][2] == 0xFF)
        {
          pad1_bits |= 0x40;
        }
        else
        {
          pad1_bits &= 0xBF;
        }
      }
    } catch(...) { }
  }
}

void NES::WriteReg4016_VS_ZAPPER(uint8 data)
{
  if(ex_controller_type == EX_VS_ZAPPER)
  {
    if((vszapper_strobe & 0x01) && !(data & 0x01))
    {
      vszapper_count = 0;
    }
    vszapper_strobe = data;
  }
}

