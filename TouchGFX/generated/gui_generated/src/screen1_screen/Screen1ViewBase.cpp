/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <touchgfx/Color.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <images/BitmapDatabase.hpp>

Screen1ViewBase::Screen1ViewBase() :
    buttonCallback(this, &Screen1ViewBase::buttonCallbackHandler)
{
    __background.setPosition(0, 0, 240, 320);
    __background.setColor(touchgfx::Color::getColorFromRGB(0, 0, 0));
    add(__background);

    textArea1.setXY(6, 10);
    textArea1.setColor(touchgfx::Color::getColorFromRGB(252, 216, 10));
    textArea1.setLinespacing(0);
    textArea1.setTypedText(touchgfx::TypedText(T___SINGLEUSE_1CQX));
    add(textArea1);

    textArea2.setXY(52, 93);
    textArea2.setColor(touchgfx::Color::getColorFromRGB(245, 12, 12));
    textArea2.setLinespacing(0);
    textArea2.setTypedText(touchgfx::TypedText(T___SINGLEUSE_16LT));
    add(textArea2);

    highestScore.setPosition(0, 119, 240, 41);
    highestScore.setColor(touchgfx::Color::getColorFromRGB(255, 0, 0));
    highestScore.setLinespacing(0);
    Unicode::snprintf(highestScoreBuffer, HIGHESTSCORE_SIZE, "%s", touchgfx::TypedText(T___SINGLEUSE_S6TU).getText());
    highestScore.setWildcard(highestScoreBuffer);
    highestScore.setTypedText(touchgfx::TypedText(T___SINGLEUSE_MS31));
    add(highestScore);

    button1.setXY(20, 208);
    button1.setBitmaps(touchgfx::Bitmap(BITMAP_START_BUTTON_VECTOR_ID), touchgfx::Bitmap(BITMAP_START_BUTTON_VECTOR_ID));
    button1.setAction(buttonCallback);
    add(button1);
}

Screen1ViewBase::~Screen1ViewBase()
{

}

void Screen1ViewBase::setupScreen()
{

}

void Screen1ViewBase::buttonCallbackHandler(const touchgfx::AbstractButton& src)
{
    if (&src == &button1)
    {
        //Interaction1
        //When button1 clicked change screen to Screen2
        //Go to Screen2 with screen transition towards East
        application().gotoScreen2ScreenCoverTransitionEast();
    }
}
