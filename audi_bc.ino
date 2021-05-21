#include <MenuConfig.h>
#include <Wire.h>
#include <GyverOLED.h>
extern "C"
{
#include "MicroMenu.h"
}
#include "PinDefinitionsAndMore.h"
//#include <irmpSelectMain15Protocols.h>  // This enables 15 main protocols
#define IRMP_SUPPORT_NEC_PROTOCOL 1 // this enables only one protocol

#ifdef ALTERNATIVE_IR_FEEDBACK_LED_PIN
#define IRMP_FEEDBACK_LED_PIN ALTERNATIVE_IR_FEEDBACK_LED_PIN
#endif
/*
 * After setting the definitions we can include the code and compile it.
 */
#include <irmp.c.h>
extern uint8_t MediumFontRus[]; // Подключаем шрифт MediumFont. Если Вы желаете использовать Кириллицу, то нужно подключить шрифт MediumFontRus.

enum ButtonValues
{
  BUTTON_NONE,
  BUTTON_UP,
  BUTTON_DOWN,
  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_ENTER,
};

GyverOLED<SSD1306_128x64, OLED_BUFFER> OLED_Big(0x3D);
GyverOLED<SSD1306_128x64, OLED_BUFFER> OLED_Small(0x3C);

IRMP_DATA irmp_data;

// подключения joystick VRX
#define PIN_VRX PA6
#define PIN_VRY PA7
#define PIN_BUTTON PB12
int joyXpr = 0;
int joyYpr = 0;
int joyX, joyY;
int angleX, angleY;

/** Example menu item specific enter callback function, run when the associated menu item is entered. */
static void Level1Item1_Enter(void)
{
  //puts("ENTER");
  OLED_Big.print("SELECT");
  OLED_Big.update();
  Serial.println("ENTER");
}

/** Example menu item specific select callback function, run when the associated menu item is selected. */
static void Level1Item1_Select(void)
{
  //puts("SELECT");
  OLED_Big.print("SELECT");
  OLED_Big.update();
  Serial.println("SELECT");
}

/** Generic function to write the text of a menu.
 *
 *  \param[in] Text   Text of the selected menu to write, in \ref MENU_ITEM_STORAGE memory space
 */
static void Generic_Write(const char *Text)
{
  if (Text)
  OLED_Big.clear();
  OLED_Big.print(Text);
  OLED_Big.update();
  Serial.println(Text);
}

// #define MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, Text)
// #define MENU_ITEM(Name, Next, Previous, Parent, Child, SelectFunc, EnterFunc, RefreshFunc, EditFunc, Text)
MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1, Level1Item1_Select, Level1Item1_Enter, "menu1");
MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL, "menu2");
MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, NULL, "menu3");
MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, NULL_MENU, NULL_MENU, NULL, NULL, "submenu1.1");
MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, NULL_MENU, NULL_MENU, NULL, NULL, "submenu1.2");

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  irmp_init();
  Serial.print(F("Ready to receive IR signals of protocols: "));
  irmp_print_active_protocols(&Serial);
#if defined(ARDUINO_ARCH_STM32)
  Serial.println(F("at pin " IRMP_INPUT_PIN_STRING));
#else
  Serial.println(F("at pin " STR(IRMP_INPUT_PIN)));
#endif

#ifdef ALTERNATIVE_IR_FEEDBACK_LED_PIN
  irmp_irsnd_LEDFeedback(true); // Enable receive signal feedback at ALTERNATIVE_IR_FEEDBACK_LED_PIN
  Serial.print(F("IR feedback pin is " STR(ALTERNATIVE_IR_FEEDBACK_LED_PIN)));
#endif

  OLED_Small.init();
  OLED_Big.init();

  OLED_Small.setScale(3);
  OLED_Small.setCursor(32, 8);
  OLED_Small.print("Start");
  OLED_Small.setScale(2);
  OLED_Small.setCursor(0, 16);
  OLED_Small.print("Start");
  OLED_Small.update();
  OLED_Big.print("AUDI");
  OLED_Big.update();
  delay(1000);

  /* Set up the default menu text write callback, and navigate to an absolute menu item entry. */
  Menu_SetGenericWriteCallback(Generic_Write);
  Menu_Navigate(&Menu_1);

} //
void loop()
{
  /* Example usage of MicroMenu - here you can create your custom menu navigation system; you may wish to perform
     * other tasks while detecting key presses, enter sleep mode while waiting for user input, etc.
     */
  switch (GetButtonPress())
  {
  case BUTTON_UP:
    Menu_Navigate((Menu_Item_t *)MENU_PREVIOUS);
    break;
  case BUTTON_DOWN:
    Menu_Navigate((Menu_Item_t *)MENU_NEXT);
    break;
  case BUTTON_LEFT:
    Menu_Navigate((Menu_Item_t *)MENU_PARENT);
    break;
  case BUTTON_RIGHT:
    Menu_Navigate((Menu_Item_t *)MENU_CHILD);
    break;
  case BUTTON_ENTER:
    Menu_EnterCurrentItem();
    break;
  default:
    if ((Menu_GetCurrentMenu() == &NULL_MENU) || (Menu_GetCurrentMenu() == NULL))
      Menu_Navigate(&Menu_1);
    OLED_Small.print(Menu_GetCurrentMenu()->Text);
    break;
  }
  delay(500);
}

void ShowDefault()
{
  OLED_Big.setCursorXY(15, 30); // курсор в (пиксель X, пиксель Y)
  OLED_Big.setScale(3);
  OLED_Big.print("Default");
  OLED_Big.update();
}
enum ButtonValues GetButtonPress()
{
  ButtonValues ret;

  if (irmp_get_data(&irmp_data))
  {
    /*
         * Skip repetitions of command
         */
    if (!(irmp_data.flags & IRMP_FLAG_REPETITION))
    {
      /*
             * Here data is available and is no repetition -> evaluate IR command
             */
      switch (irmp_data.command)
      {
      case 0x44:
        ret = BUTTON_LEFT;
        break;
      case 0x43:
        ret = BUTTON_RIGHT;
        break;
      case 0x46:
        ret = BUTTON_UP;
        break;
      case 0x15:
        ret = BUTTON_DOWN;
        break;
      case 0x09:
        ret = BUTTON_ENTER;
        break;
      default:
        ret = BUTTON_NONE;
        break;
      }
    }
    irmp_result_print(&irmp_data);
  }
  Serial.print("ret = ");
  Serial.println(ret);
  return ret;
}
