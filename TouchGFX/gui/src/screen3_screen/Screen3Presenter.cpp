#include <gui/screen3_screen/Screen3View.hpp>
#include <gui/screen3_screen/Screen3Presenter.hpp>

Screen3Presenter::Screen3Presenter(Screen3View& v)
    : view(v)
{

}

void Screen3Presenter::activate()
{

}

void Screen3Presenter::deactivate()
{

}

int Screen3Presenter::getHighestScore() const{
	return model->getHighestScore();
}
void Screen3Presenter::setHighestScore(int score){
	model->setHighestScore(score);
}
