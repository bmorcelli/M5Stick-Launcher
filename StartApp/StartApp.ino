/*            M5StickCPlus2 Simple Launcher -> Discord: @bmorcelli - Pirata#5263              */
// ========== Choose your Destiny ===========
// #define STICK_C_PLUS    // 4Mb of Flash Memory
// #define STICK_C_PLUS2   // 8Mb of Flash Memory
// #define STICK_C         // 4Mb of Flash Memory
// #define CARDPUTER       // 8Mb of Flash Memory
// ========== Flawless Victory ==============

#define LAUNCHER_VERSION "dev"

#if !defined(CARDPUTER) && !defined(STICK_C_PLUS2) && !defined(STICK_C_PLUS) && !defined(STICK_C)
  #define STICK_C_PLUS2
#endif

#if defined(STICK_C_PLUS2)
  #include <M5StickCPlus2.h>
  #include <M5Unified.h>
  #define LNDISP M5.Lcd
#endif

#if defined(STICK_C_PLUS)
  #include <M5StickCPlus.h>
  #define LNDISP M5.Lcd
#endif
#if defined(STICK_C)
  #include <M5StickC.h>
  #define LNDISP M5.Lcd
#endif
#if defined(CARDPUTER)
  #include <M5Cardputer.h>
  #define LNDISP M5Cardputer.Display
#endif

void setup() {
  #if defined(CARDPUTER)
    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
  #else
    M5.begin();
  #endif

  LNDISP.setRotation(1);
  LNDISP.setTextSize(2);
  LNDISP.fillScreen(WHITE);
  LNDISP.setCursor(0, 0);
  LNDISP.setTextColor(BLACK);
  LNDISP.println("                    ");
  LNDISP.println(" -## M5Launcher ##- ");
  LNDISP.setTextColor(RED);
  LNDISP.println("   No apps loaded   ");
  LNDISP.println("                    ");
  LNDISP.setTextColor(BLACK);
  LNDISP.println("  Turn off and on   ");
  LNDISP.println("  again to start    ");
  LNDISP.println("  the M5Launcher    ");
  LNDISP.println("                    ");
                

}

void loop() {

}
