#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"

const int MainWindow::BOARDSIZEN = BOARD_LEN;   // 棋盘“大小”，黑白棋也就是8
const int MainWindow::BOARDMARGIN = 30;         // 棋盘边缘空隙
const int MainWindow::PIECERADIUS = 15;         // 棋子（显示的）半径
const int MainWindow::MARKERSIZE = 6;           // 落子标记的长度
const int MainWindow::SQUARESIZE = 40;          // 格子（显示的）大小
const int MainWindow::POSDELTA = 20;            // 对鼠标点击进行模糊判断的误差范围

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), over_time(0), mcs_tree(nullptr),
    curr_color(Qt::black), player_color(Qt::black){

    // 弄一个随机数发生器
    static std::default_random_engine rand_eng(time(nullptr));

    // 设置棋盘大小
    setFixedSize(BOARDMARGIN * 2 + SQUARESIZE * (BOARDSIZEN - 1), BOARDMARGIN * 2 + SQUARESIZE * (BOARDSIZEN - 1));

    // 试图显示计时（非必要）
    time_show.setReadOnly(true);
    time_show.setText("0");
    layout.addWidget(&time_show);

    // 一定要开启鼠标追踪，不然不会实时更新鼠标的位置信息
    setMouseTracking(true);

    // 弹出对话框要求选择玩家和AI的先后手
    QMessageBox::StandardButton who_first = QMessageBox::information(nullptr,\
    "Who will be the first?", "The player moves first, then AI?", \
    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    who_first == QMessageBox::Yes ? player_color = Qt::black : player_color = Qt::white;

    // 初始化游戏
    InitGame();
}

MainWindow::~MainWindow(){  // 析构窗口
    mcs_tree->FreeTree();   // 删除搜索树
    if(mcs_tree){
        delete mcs_tree;    // 删除根节点
        mcs_tree = nullptr; // 置空指针
    }
}

void MainWindow::paintEvent(QPaintEvent *){   // 对窗口刷新事件的回调函数
    QPainter painter(this);
    // 绘制棋盘
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
//    QPen pen; // 线条宽度调整：美化之类的
//    pen.setWidth(2);
//    painter.setPen(pen);
    for (int i = 0; i < BOARDSIZEN; i++){
    // 绘制棋盘上的线条
        painter.drawLine(BOARDMARGIN + SQUARESIZE * i, BOARDMARGIN, BOARDMARGIN + SQUARESIZE * i, size().height() - BOARDMARGIN);
        painter.drawLine(BOARDMARGIN, BOARDMARGIN + SQUARESIZE * i, size().width() - BOARDMARGIN, BOARDMARGIN + SQUARESIZE * i);
    }

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    // 绘制动态的落子标记(防止鼠标出框越界)
    if (click_row >= 0 && click_row < BOARDSIZEN &&
        click_col >= 0 && click_col < BOARDSIZEN &&
        mcs_tree->game->PieceColorAt(click_row,click_col) == Reversi_game::EMPTY){
        brush.setColor(mcs_tree->game->WhosRound() == Reversi_game::WHITE ? Qt::white : Qt::black);
        painter.setBrush(brush);
        painter.drawRect(BOARDMARGIN + SQUARESIZE * click_col - MARKERSIZE / 2, BOARDMARGIN + SQUARESIZE * click_row - MARKERSIZE / 2, MARKERSIZE, MARKERSIZE);
    }

    // 绘制棋子，或者说更新——从游戏保存的棋盘数据里读取然后可视化棋盘
    for (int i = 0; i < BOARDSIZEN; i++)
        for (int j = 0; j < BOARDSIZEN; j++){
            if (mcs_tree->game->PieceColorAt(i,j) == Reversi_game::EMPTY) continue;
            brush.setColor(mcs_tree->game->PieceColorAt(i,j) == Reversi_game::WHITE ? Qt::white : Qt::black);
            painter.setBrush(brush);
            painter.drawEllipse(BOARDMARGIN + SQUARESIZE * j - PIECERADIUS, BOARDMARGIN + SQUARESIZE * i - PIECERADIUS, PIECERADIUS * 2, PIECERADIUS * 2);
        }

    // 绘制时间显

    // 判断输赢
    if (click_row >= 0 && click_row < BOARDSIZEN &&
            click_col >= 0 && click_col < BOARDSIZEN
//            && (mcs_tree->game->PieceColorAt(click_row, click_col) == Othello_game::WHITE ||
//             mcs_tree->game->PieceColorAt(click_row, click_col) == Othello_game::BLACK)
       )
    {   // 鼠标在棋盘里再判断输赢
        if(mcs_tree->game->GameStatus() != Reversi_game::PLAYING){  // 如果游戏状态不是PLAYING了，那么
            QString str;
            if (mcs_tree->game->GameStatus() == Reversi_game::WHITE){
                player_color == Qt::white ? str = "You Win!" : str = "You Lose!" ;
            }else if(mcs_tree->game->GameStatus() == Reversi_game::BLACK){
                player_color == Qt::black ? str = "You Win!" : str = "You Lose!" ;
            }else if(mcs_tree->game->GameStatus() == Reversi_game::DRAW){
                str = "Draw!";
            }else{
                qDebug() << "error";
            }
            QMessageBox::StandardButton btnValue = QMessageBox::information(this, "Game Result", str );
            // 重置游戏状态
            if (btnValue == QMessageBox::Ok){// 按下ok表示要重开游戏，那么进行删除和初始化
                mcs_tree->FreeTree();   // 删除搜索树
                delete mcs_tree;        // 删除根节点
                mcs_tree = nullptr;
                InitGame();             // 重新初始化游戏
            }
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event){    // 鼠标移动事件的回调函数
    int x = event->x();
    int y = event->y();

    // 棋盘边缘可以落子
    if (x > BOARDMARGIN - SQUARESIZE / 2 &&
            x < size().width() - BOARDMARGIN / 2 &&
            y > BOARDMARGIN - SQUARESIZE / 2 &&
            y < size().height()- BOARDMARGIN / 2 ){
        // 获取最靠近鼠标当前格子的左上角点的位置
        int col = x / SQUARESIZE;
        int row = y / SQUARESIZE;

        int leftTopPosX = BOARDMARGIN + SQUARESIZE * col;
        int leftTopPosY = BOARDMARGIN + SQUARESIZE * row;

        // 根据距离算出合适的点击位置，分别对应四个角上的点，根据半径距离选最近的
        click_row = -1; // 点击的行号
        click_col = -1; // 点击的列号
        int len = 0;    // 计算完后取整就可以了

        // 确定一个误差在范围内的点
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < POSDELTA){
            click_row = row;
            click_col = col;
        }
        len = sqrt((x - leftTopPosX - SQUARESIZE) * (x - leftTopPosX - SQUARESIZE) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < POSDELTA){
            click_row = row;
            click_col = col + 1;
        }
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY - SQUARESIZE) * (y - leftTopPosY - SQUARESIZE));
        if (len < POSDELTA){
            click_row = row + 1;
            click_col = col;
        }
        len = sqrt((x - leftTopPosX - SQUARESIZE) * (x - leftTopPosX - SQUARESIZE) + (y - leftTopPosY - SQUARESIZE) * (y - leftTopPosY - SQUARESIZE));
        if (len < POSDELTA){
            click_row = row + 1;
            click_col = col + 1;
        }
    }// 存了坐标以后也要重绘
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *){  // 鼠标释放事件的回调函数（对应落子）
    if ( game_type == Reversi_game::PVP || player_color == curr_color ){
        // 如果对手是人，那么不管现在谁下，都要调用人下
        // 如果对手是AI，但他已经下完，就该轮到我们
        OneMoveHuman();
        // 如果是PVE模式，人下完了接着调用一次AI下
        if ( game_type == Reversi_game::PVE && player_color != curr_color ){
            OneMoveAI();
        }
    }
}

void MainWindow::timerEvent(QTimerEvent *){ //  用于确定超时的计时器
    if(over_time >= 60){
        mcs_tree->game->TimeOut();
        update();
    }
    over_time ++ ;
    time_show.setText(QString(over_time));
}

void MainWindow::OneMoveHuman(){    // “人工”下棋
    if (click_row != -1 && click_col != -1 && mcs_tree->game->PieceColorAt(click_row, click_col) == Reversi_game::EMPTY){
    // 只有有效点击且该处没有子，才下子
        mcs_tree = mcs_tree->Play(click_row, click_col);    // 每次更新需要捕获新树，旧树已经删除
        curr_color = mcs_tree->game->WhosRound() == Reversi_game::WHITE ? Qt::white : Qt::black;
        over_time = 0;      // 人下完重置计时器
        update();           // 重绘
    }
    // 弃权功能？
}

void MainWindow::OneMoveAI(){       // AI下棋
    mcs_tree = mcs_tree->SearchAndPlay();   // 每次更新需要捕获新树，旧树已经删除
    mcs_tree->game->PrintBoard();
    curr_color = mcs_tree->game->WhosRound() == Reversi_game::WHITE ? Qt::white : Qt::black;
    over_time = 0;          // AI下完重置闹钟
    update();
}

void MainWindow::InitGame(){    // 初始化游戏
    total_time = 0.0;   // 总时间归零
    InitPVE();          // 初始化PVE（默认）
    over_time = 0;      // 设置计时器
    startTimer(1000);   // 定时器精确度1s
}

void MainWindow::InitPVP(){     // 初始化PVP模式游戏
    game_type = Reversi_game::PVP;
    // do something...
    update();
}

void MainWindow::InitPVE(){     // 初始化PVE模式游戏
    game_type = Reversi_game::PVE;
    Reversi_game* othello = new Reversi_game();
    othello->PrintBoard();      // 打印棋盘主要用于调试
    mcs_tree =  new MCST_node(othello);
    if(player_color == Qt::white) OneMoveAI();  // 如果AI先手，那么先让AI下一次
    update();
}
