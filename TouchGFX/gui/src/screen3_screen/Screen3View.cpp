#include <gui/screen3_screen/Screen3View.hpp>
#include <touchgfx/Color.hpp>
#include "cmsis_os.h"
#include "main.h"

extern osMessageQueueId_t movingQueueHandle;
extern osMessageQueueId_t levelQueueHandle;
extern uint8_t currScreen;

/**
 * @brief	TouchGFX yêu cầu RGB888 để xử lý đúng màu (TetrisEngine dùng RGB565 để tiết kiệm bộ nhớ)
 * @param	rgb565: uint16_t màu cần chuyển sang RGB888
 * @param	r: uint8_t& biểu diễn màu đỏ
 * @param	g: uint8_t& biểu diễn màu xanh lá cây
 * @param	b: uint8_t& biểu diễn màu xanh nước biển
 * @retval	None
 */
static void convertRGB565ToRGB888(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = ((rgb565 >> 11) & 0x1F) << 3; // 5-bit red => 8-bit
    g = ((rgb565 >> 5) & 0x3F) << 2;  // 6-bit green => 8-bit
    b = (rgb565 & 0x1F) << 3;         // 5-bit blue => 8-bit
}

Screen3View::Screen3View()
{
	//khởi tạo lưới chính
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            int px = x * 16;
            int py = y * 16;
            colBoxes[y][x].setBorderColor(Color::getColorFromRGB(0, 0, 0));
            colBoxes[y][x].setBorderSize(1);
            colBoxes[y][x].setPosition(px, py, 16, 16);
            colBoxes[y][x].setVisible(true);
            add(colBoxes[y][x]);
        }
    }

    //khởi tạo lưới để hiện thị next block
	int previewX = 200;
	int previewY = 90;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			int px = previewX + x * 8;
			int py = previewY + y * 8;
			previewBoxes[y][x].setAlpha(0);
			previewBoxes[y][x].setBorderSize(1);
			previewBoxes[y][x].setPosition(px, py, 8, 8);
			previewBoxes[y][x].setVisible(true);
			add(previewBoxes[y][x]);
		}
	}

	currScreen = 3;
	image2.setVisible(false);
	musicGameOver = false;
    DF_SendCommand(0x0F, 0x02, 0x01);

    char res = 'a';
    //loại bỏ hết các phần tử trong moving queue
    while(osMessageQueueGetCount(movingQueueHandle) > 0){
    	osMessageQueueGet(movingQueueHandle, &res, NULL, 10);
    }

    level = 20;
    while(osMessageQueueGetCount(levelQueueHandle) > 1){
    	osMessageQueueGet(levelQueueHandle, &res, NULL, 10);
    }
    if(osMessageQueueGetCount(levelQueueHandle) > 0){
    	osMessageQueueGet(levelQueueHandle, &res, NULL, 10);
    }

    if(res == 'e') level = 20;
    else if(res == 'm') level = 15;
    else if(res == 'h') level = 10;
}

void Screen3View::setupScreen()
{
    Screen3ViewBase::setupScreen();
	engine.init();
	tickCount = 0;
	image2.setVisible(false);
	image2.invalidate();
}

void Screen3View::tearDownScreen()
{
    Screen3ViewBase::tearDownScreen();
}


/**
 * @brief	Xử lý game sau mỗi tick event (tương đương vòng loop để xử lý game)
 * @retval	None
 */
void Screen3View::handleTickEvent()
{
    if (++tickCount % level == 0) {
		if(engine.isGameOver()) {	//kiểm tra gameover

			//set các ô trên lưới là ko hiển thị nếu gameover
			for(int y = 0; y < GRID_HEIGHT; y++) {
				for(int x = 0; x < GRID_WIDTH; x++) {
					colBoxes[y][x].setVisible(false);
					colBoxes[y][x].invalidate();
				}
			}

			//set các ô trên pre boxes là ko hiển thị nếu gameover
			for (int y = 0; y < 4; y++) {
				for (int x = 0; x < 4; x++) {
					previewBoxes[y][x].setVisible(false);
					previewBoxes[y][x].invalidate();
				}
			}
			textArea1.setVisible(false);
			textArea2.setVisible(false);
			score.setVisible(false);

			image2.setVisible(true);
			image2.invalidate();
			presenter->setHighestScore(engine.getScore());	//cập nhật highest score
			return;
		}

		//kiểm tra queue -> có phần tử -> có tín hiệu nút bấm điều khiển
    	if(osMessageQueueGetCount(movingQueueHandle) > 0){
    		char res;
    		osMessageQueueGet(movingQueueHandle, &res, NULL, 10);
    		if(res == 'L') engine.moveLeft();
    		else if(res == 'R') engine.moveRight();
    		else if(res == 'T') engine.rotate();
    		else if(res == 'D') engine.drop();
    		osThreadNew(SingleBeepTask, NULL, NULL);
    	}
    	//update hiển thị
        engine.update();
        Unicode::snprintf(scoreBuffer, SCORE_SIZE, "%d", engine.getScore());
        score.invalidate();
        presenter->setHighestScore(engine.getScore());

        if(engine.getTakeScore()){	//nếu ghi được điểm -> tạo tiếng beep - beep
        	osThreadNew(DoubleBeepTask, NULL, NULL);
        	engine.setTakeScore(false);
        }
        if(engine.isGameOver() && musicGameOver == false){ //game over -> bật nhạc game over + ghi điểm vào flash
        	musicGameOver = true;
        	osThreadNew(GameOverTask, NULL, NULL);
        	uint32_t tmp = engine.getScore();
//        	uint32_t tmp = 21;
        	if((uint32_t)presenter->getHighestScore() <= tmp){
        		Flash_Write_Data(0x081E0000, &tmp, 1);
        	}
        }
        //vẽ lại lưới chính
        drawGrid();

        //vẽ lại lưới cho pre block
        drawPreview();
    }
}

/**
 * @brief	Vẽ lưới game
 * @retval	None
 */
void Screen3View::drawGrid(){
//    const auto& grid = engine.getGrid();
    const auto& block = engine.getCurrentBlock();
    int currX = engine.getCurrX();
    int currY = engine.getCurrY();

    //Vẽ lưới chính
    for (int y = 0; y < GRID_HEIGHT; ++y) {
        for (int x = 0; x < GRID_WIDTH; ++x) {
//            colBoxes[y][x].setColor(grid[y][x] ? Color::getColorFromRGB(255, 0, 255)
//                                               : Color::getColorFromRGB(0, 0, 0));
        	uint16_t gridColor = engine.getGridColor(x, y);
			uint8_t r, g, b;
			convertRGB565ToRGB888(gridColor, r, g, b);
			colBoxes[y][x].setColor(gridColor ? Color::getColorFromRGB(r, g, b)
											 : Color::getColorFromRGB(0, 0, 0));
			colBoxes[y][x].setAlpha(255);
            colBoxes[y][x].invalidate();
        }
    }

    // Vẽ block đang rơi
    uint16_t blockColor = engine.getCurrentBlockColor();
	uint8_t r, g, b;
	convertRGB565ToRGB888(blockColor, r, g, b);

	//lấy bound của block (hình chữ nhật nhỏ nhất mà chứa được toàn bộ block)
    int minX, maxX, minY, maxY;
    engine.getBlockBounds(block, minX, maxX, minY, maxY);
    for (int i = minY; i <= maxY; ++i)
        for (int j = minX; j <= maxX; ++j)
            if (block[i][j]) {
            	//vị trí thực của ô
                int gx = currX + j;
                int gy = currY + i;
                if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
                    colBoxes[gy][gx].setColor(Color::getColorFromRGB(r, g, b));
                    colBoxes[gy][gx].setAlpha(255);
                    colBoxes[gy][gx].invalidate();
                }
            }
}

/**
 * @brief	Vẽ pre block
 * @retval	None
 */
void Screen3View::drawPreview() {
    // Lấy khối tiếp theo
    TetrisEngine::BlockMatrix nextBlock;
    int nextBlockSize;
    uint16_t nextBlockColor;
    engine.getNextBlock(nextBlock, nextBlockSize, nextBlockColor);

    // Đặt lại previewBox trước về trạng thái trong suốt
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			previewBoxes[i][j].setAlpha(0);
			previewBoxes[i][j].invalidate();
		}
	}

	// Vẽ preview block
	uint8_t r, g, b;
	convertRGB565ToRGB888(nextBlockColor, r, g, b);

	int minX, maxX, minY, maxY;
	engine.getBlockBounds(nextBlock, minX, maxX, minY, maxY);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (nextBlock[i][j]) {
				int px = j - minX;
				int py = i - minY;
				if (px < 4 && py < 4 && px >= 0 && py >= 0) {
					previewBoxes[py][px].setColor(Color::getColorFromRGB(r, g, b));
					previewBoxes[py][px].setAlpha(255);
					previewBoxes[py][px].invalidate();
				}
			}
		}
	}
}
