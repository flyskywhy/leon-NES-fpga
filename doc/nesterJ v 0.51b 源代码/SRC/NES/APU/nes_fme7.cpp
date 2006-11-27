/*
** Sunsoft FME7 ExSound by TAKEDA, toshiya
**
** original: s_fme7.c in nezp0922
*/

static int8 env_pulse[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+00,
};
static int8 env_pulse_hold[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
	0x1F,+00,
};
static int8 env_saw[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-62,
};
static int8 env_tri[] = 
{
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,+02,
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-126,
};
static int8 env_xpulse[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+00,
};
static int8 env_xpulse_hold[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
	0x00,+00,
};
static int8 env_xsaw[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,-62,
};

static int8 env_xtri[] = 
{
	0x00,+02,0x01,+02,0x02,+02,0x03,+02,0x04,+02,0x05,+02,0x06,+02,0x07,+02,
	0x08,+02,0x09,+02,0x0A,+02,0x0B,+02,0x0C,+02,0x0D,+02,0x0E,+02,0x0F,+02,
	0x10,+02,0x11,+02,0x12,+02,0x13,+02,0x14,+02,0x15,+02,0x16,+02,0x17,+02,
	0x18,+02,0x19,+02,0x1A,+02,0x1B,+02,0x1C,+02,0x1D,+02,0x1E,+02,0x1F,+02,
	0x1F,+02,0x1E,+02,0x1D,+02,0x1C,+02,0x1B,+02,0x1A,+02,0x19,+02,0x18,+02,
	0x17,+02,0x16,+02,0x15,+02,0x14,+02,0x13,+02,0x12,+02,0x11,+02,0x10,+02,
	0x0F,+02,0x0E,+02,0x0D,+02,0x0C,+02,0x0B,+02,0x0A,+02,0x09,+02,0x08,+02,
	0x07,+02,0x06,+02,0x05,+02,0x04,+02,0x03,+02,0x02,+02,0x01,+02,0x00,-126,
};

static int8 *(env_table[16]) =
{
	env_pulse,	env_pulse,	env_pulse,	env_pulse,
	env_xpulse,	env_xpulse,	env_xpulse,	env_xpulse,
	env_saw,	env_pulse,	env_tri,	env_pulse_hold,
	env_xsaw,	env_xpulse,	env_xtri,	env_xpulse_hold,
};

static uint32 PSGSoundNoise(PSG_NOISE *ch)
{
	if (ch->update)
	{
		if (ch->update & 1)
		{
			ch->spd = ((ch->regs[0] & 0x1F) + 1) << 18;
		}
		ch->update = 0;
	}

	if (ch->spd)
	{
		ch->cycles -= ch->cps;
		while (ch->cycles < 0)
		{
			ch->cycles += ch->spd;
			if ((ch->noiserng + 1) & 2) ch->noiseout = ~ch->noiseout;
			if (ch->noiserng & 1) ch->noiserng ^= 0x28000;
			ch->noiserng >>= 1;
		}
	}
	return ch->noiseout;
}

static uint32 PSGSoundEnvelope(PSG_ENVELOPE *ch)
{
	if (ch->update)
	{
		if (ch->update & (1 | 2))
		{
			ch->spd = ((ch->regs[1] << 8) + ch->regs[0] + 1) << 18;
		}
		if (ch->update & 4)
		{
			ch->adr = env_table[ch->regs[2] & 0xF];
		}
		ch->update = 0;
	}

	if (ch->spd)
	{
		ch->cycles -= ch->cps;
		while (ch->cycles < 0)
		{
			ch->cycles += ch->spd;
			ch->adr += ch->adr[1];
		}
		if (ch->adr[0] & 0x1F)
		{
			ch->envout = (0x1F - (ch->adr[0] & 0x1F)) << (LOG_BITS - 2 + 1);
			ch->envout += apu->psg.mastervolume;
			ch->envout = LogToLinear(ch->envout, LOG_LIN_BITS - 21);
		}
		else
			ch->envout = 0;
	}
	return ch->envout;
}

static int32 PSGSoundSquare(PSG_SQUARE *ch)
{
	int32 output = 0;
	if (ch->update)
	{
		if (ch->update & (1 | 2))
		{
			ch->spd = (((ch->regs[1] & 0x0F) << 8) + ch->regs[0] + 1) << 18;
		}
		ch->update = 0;
	}

	if (!ch->spd) return 0;

	ch->cycles -= ch->cps;
	while (ch->cycles < 0)
	{
		ch->cycles += ch->spd;
		ch->adr++;
	}

	if (!ch->mute && ch->key)
	{
		uint32 volume;
		if (ch->regs[2] & 0x10)
			volume = apu->psg.envelope.envout;
		else if (ch->regs[2] & 0xF)
		{
			volume = (0xF - (ch->regs[2] & 0xF)) << (LOG_BITS - 1 + 1);
			volume += apu->psg.mastervolume;
			volume = LogToLinear(volume, LOG_LIN_BITS - 21);
		}
		else
			volume = 0;
		if (ch->key & 1)
		{
			if (ch->adr & 1)
				output += volume;
			else
				output -= volume;
		}
		if (ch->key & 2)
		{
			if (apu->psg.noise.noiseout)
				output += volume;
			else
				output -= volume;
		}
	}
	return output;
}

static int32 PSGSoundRender(void)
{
	int32 accum = 0;
	PSGSoundNoise(&apu->psg.noise);
	PSGSoundEnvelope(&apu->psg.envelope);
	accum += PSGSoundSquare(&apu->psg.square[0]);
	accum += PSGSoundSquare(&apu->psg.square[1]);
	accum += PSGSoundSquare(&apu->psg.square[2]);
	return accum;
}

static void PSGSoundVolume(uint32 volume)
{
	apu->psg.mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

static void PSGSoundWrireAddr(uint32 address, uint8 value)
{
	apu->psg.adr = value;
}

static void PSGSoundWrireData(uint32 address, uint8 value)
{
	switch (apu->psg.adr)
	{
		case 0x0: case 0x1:
		case 0x2: case 0x3:
		case 0x4: case 0x5:
			apu->psg.square[apu->psg.adr >> 1].regs[apu->psg.adr & 1] = value;
			apu->psg.square[apu->psg.adr >> 1].update |= 1 << (apu->psg.adr & 1);
			break;
		case 0x6:
			apu->psg.noise.regs[0] = value;
			apu->psg.noise.update |= 1 << 0;
			break;
		case 0x7:
			{
				uint8 ch;
				for (ch = 0; ch < 3; ch++)
				{
					apu->psg.square[ch].key = 0;
					if (!(value & (1 << ch))) apu->psg.square[ch].key |= 1;
					if (!(value & (8 << ch))) apu->psg.square[ch].key |= 2;
				}
			}
			break;
		case 0x8: case 0x9: case 0xA:
			apu->psg.square[apu->psg.adr & 3].regs[2] = value;
			apu->psg.square[apu->psg.adr & 3].update |= 1 << 2;
			break;
		case 0xB: case 0xC: case 0xD:
			apu->psg.envelope.regs[apu->psg.adr - 0xB] = value;
			apu->psg.envelope.update |= 1 << (apu->psg.adr - 0xB);
			break;
	}
}

static void PSGSoundWrite(uint32 address, uint8 value)
{
	if (address == 0xC000)
	{
		PSGSoundWrireAddr(address, value);
	}
	else if (address == 0xE000)
	{
		PSGSoundWrireData(address, value);
	}
}

static void PSGSoundSquareReset(PSG_SQUARE *ch)
{
	ch->cps = DivFix(NES_BASECYCLES, 6 * 8 * 4 * SAMPLE_RATE, 18);
}

static void PSGSoundNoiseReset(PSG_NOISE *ch)
{
	ch->cps = DivFix(NES_BASECYCLES, 6 * 8 * 4 * SAMPLE_RATE, 18);
	ch->noiserng = 1;
	ch->noiseout = 0xFF;
}

static void PSGSoundEnvelopeReset(PSG_ENVELOPE *ch)
{
	ch->cps = DivFix(NES_BASECYCLES, 6 * 8 * 4 * SAMPLE_RATE, 18);
	ch->adr = env_table[0];
}

static void PSGSoundReset(void)
{
	memset(&apu->psg, 0, sizeof(PSGSOUND));
	PSGSoundNoiseReset(&apu->psg.noise);
	PSGSoundEnvelopeReset(&apu->psg.envelope);
	PSGSoundSquareReset(&apu->psg.square[0]);
	PSGSoundSquareReset(&apu->psg.square[1]);
	PSGSoundSquareReset(&apu->psg.square[2]);
}

