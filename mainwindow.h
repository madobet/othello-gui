#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/*
 * 待添加的功能：
 * 1. PVP
 * 2. 弃权让电脑随意帮我下
 * 3. 悔棋
 * 4. 暂停/存档/加载
*/

#include <QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QLineEdit>
#include <QGridLayout>
#include"MCST_node.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *);             // 重绘事件
    void mouseMoveEvent(QMouseEvent *event);    // 监听鼠标移动事件
    void mouseReleaseEvent(QMouseEvent *);      // 实际落子（对应鼠标释放）
    void timerEvent(QTimerEvent *);             // 计时器事件（处理超时）

private:
    // 绘制计时器，待修正
    QGridLayout layout;
    QLineEdit time_show;

    static const int BOARDSIZEN;    // 棋盘大小，黑白棋为8
    static const int BOARDMARGIN;   // 棋盘边缘空隙
    static const int PIECERADIUS;   // 棋子半径
    static const int MARKERSIZE;    // 落子标记边长
    static const int SQUARESIZE;    // 格子大小
    static const int POSDELTA;      // 鼠标点击的模糊距离上限
    unsigned over_time;             // 超时计时变量，秒为单位
    MCST_node *mcs_tree;            // 游戏指针
    QColor curr_color;              // 当前的“颜色”，轮到谁下
    QColor player_color;            // 玩家执子的颜色
    Reversi_game::Game_type game_type;  // 游戏类型：PVP或PVE
    int click_row, click_col;       // 存储点击位置
    void InitGame();                // 初始化游戏

private slots:
    void OneMoveHuman();    // 人下棋
    void OneMoveAI();       // AI下棋

    void InitPVP();         // 初始化PVP游戏
    void InitPVE();         // 初始化PVE游戏
};

#endif // MAINWINDOW_H
