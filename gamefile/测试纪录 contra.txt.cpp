
27.65 MHz,2KB DCache:
6+A+P: 82677;   Frame: 266;
6+A: 25287;     Frame: 267;
6+A: 24969;     Frame: 268;
6+A: 24912;     Frame: 269;
6+A: 24912;     Frame: 270;
6+A: 24915;     Frame: 271;
6+A: 24915;     Frame: 272;
6+A+P: 82386;   Frame: 273;

/*
将memcpy一条扫描线到SDRAM的方法改为直接操作SDRAM中的每个像素，可见速度明显变慢，不可取
6+A+P: 109953;  Frame: 266;
6+A: 25287;     Frame: 267;
6+A: 24966;     Frame: 268;
6+A: 24912;     Frame: 269;
6+A: 24912;     Frame: 270;
6+A: 24915;     Frame: 271;
6+A: 24918;     Frame: 272;
6+A+P: 109653;  Frame: 273;
*/

/*开启killwif，可以看到速度稍有提升，与TSIM中的现象相反，只是提升有限，只有50微秒
6+A+P: 82623;   Frame: 266;
6+A: 25254;     Frame: 267;
6+A: 24921;     Frame: 268;
6+A: 24882;     Frame: 269;
6+A: 24882;     Frame: 270;
6+A: 24882;     Frame: 271;
6+A: 24885;     Frame: 272;
6+A+P: 82374;   Frame: 273;

关闭killwif，*/text由RAM移到SDRAM中，速度稍有提升
6+A+P: 82605;   Frame: 266;
6+A: 25260;     Frame: 267;
6+A: 24912;     Frame: 268;
6+A: 24873;     Frame: 269;
6+A: 24876;     Frame: 270;
6+A: 24876;     Frame: 271;
6+A: 24876;     Frame: 272;
6+A+P: 82365;   Frame: 273;

/*再开启killwif，可以看到速度稍有下降，可见killwif没有什么用处，再考虑到代码的简洁，可以将killwif和killif之类的全部去除
6+A+P: 82641;   Frame: 266;
6+A: 25260;     Frame: 267;
6+A: 24948;     Frame: 268;
6+A: 24891;     Frame: 269;
6+A: 24888;     Frame: 270;
6+A: 24891;     Frame: 271;
6+A: 24891;     Frame: 272;
6+A+P: 82389;   Frame: 273;

在开启killwif的状态下，开启DMA_SDRAM
6+A+P: 74517;   Frame: 266;
6+A: 25263;     Frame: 267;
6+A: 24945;     Frame: 268;
6+A: 24888;     Frame: 269;
6+A: 24891;     Frame: 270;
6+A: 24888;     Frame: 271;
6+A: 24894;     Frame: 272;
6+A+P: 74244;   Frame: 273;*/


在关闭killwif的状态下，开启DMA_SDRAM，可见通过DMA写SDRAM的方式每桢可节省8毫秒左右
6+A+P: 74448;   Frame: 266;
6+A: 25260;     Frame: 267;
6+A: 24912;     Frame: 268;
6+A: 24876;     Frame: 269;
6+A: 24876;     Frame: 270;
6+A: 24873;     Frame: 271;
6+A: 24876;     Frame: 272;
6+A+P: 74217;   Frame: 273;

------------------------------------------------------
2KB Cache，开启DMA_SDRAM
6+A+P: 76923;   Frame: 266;
6+A: 25260;     Frame: 267;
6+A: 24912;     Frame: 268;
6+A: 24873;     Frame: 269;
6+A: 24873;     Frame: 270;
6+A: 24876;     Frame: 271;
6+A: 24876;     Frame: 272;
6+A+P: 76512;   Frame: 273;

4KB Cache，开启DMA_SDRAM，可见Cache由2KB升至4KB后时间可以缩短200~500微秒，速度提升0.76%
6+A+P: 76428;   Frame: 266;
6+A: 25113;     Frame: 267;
6+A: 24687;     Frame: 268;
6+A: 24660;     Frame: 269;
6+A: 24660;     Frame: 270;
6+A: 24663;     Frame: 271;
6+A: 24660;     Frame: 272;
6+A+P: 76080;   Frame: 273;

40.5 MHz，4KB Cache，可见速度与频率成正比
6+A+P: 50954;   Frame: 266
6+A: 16740;     Frame: 267
6+A: 16458;     Frame: 268
6+A: 16440;     Frame: 269
6+A: 16440;     Frame: 270
6+A: 16442;     Frame: 271
6+A: 16442;     Frame: 272
6+A+P: 50714;   Frame: 273

-------------------------------------------------------------
40.5 MHz，4KB Cache:
/*关闭killtable，g_byTestTable放在DTCM中，可见速度会变慢
6+A+P: 50982;   Frame: 266;
6+A: 16756;     Frame: 267;
6+A: 16472;     Frame: 268;
6+A: 16454;     Frame: 269;
6+A: 16452;     Frame: 270;
6+A: 16454;     Frame: 271;
6+A: 16454;     Frame: 272;
6+A+P: 50724;   Frame: 273;

关闭killtable，g_byTestTable放在ITCM中，可见速度会更慢，考虑到DTCM回用RAM来代替及使用表格所占的DTCM的容量，可以总是开启killtable了
6+A+P: 51416;   Frame: 266;
6+A: 17200;     Frame: 267;
6+A: 16918;     Frame: 268;
6+A: 16900;     Frame: 269;
6+A: 16900;     Frame: 270;
6+A: 16902;     Frame: 271;
6+A: 16900;     Frame: 272;
6+A+P: 51174;   Frame: 273;*/

/*取消对nes_SP之类的寄存器锁定，慢
6+A+P: 51856;   Frame: 266;
6+A: 17612;     Frame: 267;
6+A: 17326;     Frame: 268;
6+A: 17308;     Frame: 269;
6+A: 17308;     Frame: 270;
6+A: 17308;     Frame: 271;
6+A: 17308;     Frame: 272;
6+A+P: 51586;   Frame: 273;*/

由于g1和g3不能用，将应该不常用的nes_Y的g5给nes_pc，速度提升了2毫秒！ITCM减少了2800个字节
6+A+P: 49046;   Frame: 266;
6+A: 14760;     Frame: 267;
6+A: 14476;     Frame: 268;
6+A: 14458;     Frame: 269;
6+A: 14456;     Frame: 270;
6+A: 14456;     Frame: 271;
6+A: 14456;     Frame: 272;
6+A+P: 48728;   Frame: 273;

将nes_SP的g7给lastbank，速度提升了100微秒，
6+A+P: 48948;   Frame: 266;
6+A: 14638;     Frame: 267;
6+A: 14354;     Frame: 268;
6+A: 14332;     Frame: 269;
6+A: 14330;     Frame: 270;
6+A: 14334;     Frame: 271;
6+A: 14332;     Frame: 272;
6+A+P: 48604;   Frame: 273;

killif3
以查找数据指针数组的方式减少6502代码中Read6502RAM的条件分支
6+A+P: 48952;   Frame: 266;
6+A: 14638;     Frame: 267;
6+A: 14352;     Frame: 268;
6+A: 14330;     Frame: 269;
6+A: 14330;     Frame: 270;
6+A: 14332;     Frame: 271;
6+A: 14332;     Frame: 272;
6+A+P: 48604;   Frame: 273;

完全killif3的范围，可见速度稍快
6+A+P: 48910;   Frame: 266;
6+A: 14616;     Frame: 267;
6+A: 14328;     Frame: 268;
6+A: 14308;     Frame: 269;
6+A: 14308;     Frame: 270;
6+A: 14308;     Frame: 271;
6+A: 14310;     Frame: 272;
6+A+P: 48570;   Frame: 273;

-----------------------------------------------------
添加InfoNES_System_LEON后,速度有增有减,事实上跳桢数还稍有下降
6+A+P: 47828;   Frame: 266;
6+A: 15080;     Frame: 267;
6+A: 14792;     Frame: 268;
6+A: 14774;     Frame: 269;
6+A: 14774;     Frame: 270;
6+A: 14776;     Frame: 271;
6+A: 14776;     Frame: 272;
6+A+P: 47574;   Frame: 273;

整合源文件之后
6+A+P: 47826;   Frame: 266;
6+A: 15078;     Frame: 267;
6+A: 14792;     Frame: 268;
6+A: 14774;     Frame: 269;
6+A: 14772;     Frame: 270;
6+A: 14776;     Frame: 271;
6+A: 14776;     Frame: 272;
6+A+P: 47576;   Frame: 273;
-----------------------------------------------------
添加手柄控制，进入游戏有声音后，30MHz，4KB ICache，8KB DCache
6+A+P: 60285;
6+A: 20968;
6+A: 20946;
6+A: 20930;
6+A: 20992;
6+A: 20277;
6+A: 21213;
标题画面
6+A+P: 61256;   Frame: 266;
6+A: 20061;     Frame: 267;
6+A: 19821;     Frame: 268;
6+A: 19824;     Frame: 269;
6+A: 19818;     Frame: 270;
6+A: 19821;     Frame: 271;
6+A: 19824;     Frame: 272;
6+A+P: 60925;   Frame: 273;


2KB ICache，4KB DCache，可见Cache由4KB升至8KB后时间可以缩短869~1125微秒，速度提升1.8%
6+A+P: 61154;
6+A: 21560;
6+A: 21226;
6+A: 21277;
6+A: 20426;
6+A: 20333;
6+A: 20288;
标题画面
6+A+P: 62381;   Frame: 266;
6+A: 20197;     Frame: 267;
6+A: 19858;     Frame: 268;
6+A: 19834;     Frame: 269;
6+A: 19837;     Frame: 270;
6+A: 19840;     Frame: 271;
6+A: 19840;     Frame: 272;
6+A+P: 62101;   Frame: 273;


-----------------------------------------------------
6+A+P: 62330;   Frame: 266;
6+A: 20197;     Frame: 267;
6+A: 19856;     Frame: 268;
6+A: 19840;     Frame: 269;
6+A: 19834;     Frame: 270;
6+A: 19837;     Frame: 271;
6+A: 19840;     Frame: 272;
6+A+P: 62098;   Frame: 273;

/*PPU_DrawLine由处理33个tile改为32个，时间节省560微秒，速度提升0.9%，考虑到麻将游戏要显示左右两边的8个像素而不能只处理32个tile来只显示每行240个像素，这么点速度提升不要也罢。
6+A+P: 61770;   Frame: 266;
6+A: 20197;     Frame: 267;
6+A: 19856;     Frame: 268;
6+A: 19837;     Frame: 269;
6+A: 19834;     Frame: 270;
6+A: 19834;     Frame: 271;
6+A: 19840;     Frame: 272;*/

/*将写2007移到最前,反而稍有慢
6+A+P: 62397;   Frame: 266;
6+A: 20200;     Frame: 267;
6+A: 19856;     Frame: 268;
6+A: 19834;     Frame: 269;
6+A: 19834;     Frame: 270;
6+A: 19837;     Frame: 271;
6+A: 19842;     Frame: 272;
6+A+P: 62141;   Frame: 273;*/

使用处理最左边8个像素的代码,时间增加78微秒,速度减慢0.12%,考虑使用之
6+A+P: 62408;   Frame: 266;
6+A: 20208;     Frame: 267;
6+A: 19858;     Frame: 268;
6+A: 19834;     Frame: 269;
6+A: 19834;     Frame: 270;
6+A: 19837;     Frame: 271;
6+A: 19840;     Frame: 272;
6+A+P: 62176;   Frame: 273;

ITCM:7158
inline SLNES_PadState之后,时间减少27微秒,无甚用
ITCM:7158
6+A+P: 62381;   Frame: 266;
6+A: 20197;     Frame: 267;
6+A: 19858;     Frame: 268;
6+A: 19840;     Frame: 269;
6+A: 19837;     Frame: 270;
6+A: 19837;     Frame: 271;
6+A: 19840;     Frame: 272;
6+A+P: 62141;   Frame: 273;


6+A+P: 62413;   Frame: 266;
6+A: 20202;     Frame: 267;
6+A: 19858;     Frame: 268;
6+A: 19840;     Frame: 269;
6+A: 19834;     Frame: 270;
6+A: 19837;     Frame: 271;
6+A: 19837;     Frame: 272;


2KB ICache，2KB DCache，可见Cache由2KB升至4KB后时间可以缩短325微秒，速度提升0.52%
6+A+P: 62738;   Frame: 266;
6+A: 20338;     Frame: 267;
6+A: 20061;     Frame: 268;
6+A: 20032;     Frame: 269;
6+A: 20021;     Frame: 270;
6+A: 20021;     Frame: 271;
6+A: 20032;     Frame: 272;
6+A+P: 62394;   Frame: 273;

连续同一条件测试,可见误差范围是40微秒以下
6+A+P: 62722;   Frame: 266;
6+A+P: 62706;   Frame: 266;
6+A+P: 62706;   Frame: 266;
6+A+P: 62696;   Frame: 266;
6+A+P: 62696;   Frame: 266;
6+A+P: 62698;   Frame: 266;
6+A+P: 62728;   Frame: 266;


/*inline CPU_NMI之后,无甚用
ITCM:72b8
6+A+P: 62741;   Frame: 266;
6+A: 20344;     Frame: 267;
6+A: 20058;     Frame: 268;
6+A: 20021;     Frame: 269;
6+A: 20037;     Frame: 270;
6+A: 20026;     Frame: 271;
6+A: 20024;     Frame: 272;
6+A+P: 62381;   Frame: 273;*/

static inline APU_Enqueue和APU_Dequeue之后,除了减少了48个字节,无甚用
ITCM:7128
6+A+P: 62738;   Frame: 266;
6+A: 20357;     Frame: 267;
6+A: 20080;     Frame: 268;
6+A: 20037;     Frame: 269;
6+A: 20029;     Frame: 270;
6+A: 20032;     Frame: 271;
6+A: 20040;     Frame: 272;
6+A+P: 62405;   Frame: 273;