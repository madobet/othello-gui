#ifndef OTHELLO_GAME_H
#define OTHELLO_GAME_H

#include <iostream>
#include <vector>
#include <random>
#include <memory.h>

#define MOVE_BITS 3     // 右移左移取代乘除加快速度
#define BOARD_LEN 8
#define BOARD_SIZE 64

// class that record a possibile playing position
// along with the number of chesses that should be reversed in each direction if the position is played
class One_move {    // One possible move
public:
    One_move(int pos, const int* rev_n ):move_pos(pos) {
        for( int i = 0; i < BOARD_LEN; i++) rev_n_of_dir[i] = rev_n[i];
    }
    // where to put the piece
    const int move_pos;
    // how many pieces of competitor at each direction will be reversed
    // when put piece at move_pos
    int rev_n_of_dir[BOARD_LEN] = {};
};

class Reversi_game {
public:
    enum Game_type {
        PVP,
        PVE
    };
    Reversi_game();

    // parameters that record the state of a position on the curr_position
    // or the state of the game
    static const int BLACK;
    static const int WHITE;
    static const int EMPTY;
    static const int DRAW;
    static const int PLAYING;
    static const int INVALID;
    static const int DIRSET[BOARD_LEN][2];

    std::vector<One_move> moves_set;    // 所有可行的下一步落子方式集

    static inline int Posof(int x, int y) {  return x * BOARD_LEN + y; }    // 棋盘x行y列对应的位置编号
    static inline int Xof(int pos) {  return pos >> MOVE_BITS; }            // pos位置的x（行号）
    static inline int Yof(int pos) {  return pos % BOARD_LEN;  }            // pos位置的y（列号）

    inline int WhosRound() { return curr_round; }       // 现在是谁的回合？
    inline int GameStatus() { return game_status; }     // 游戏当前的进行状态？
    inline int PieceColorAt(int row, int col) {         // row行，col列上的棋子是什么颜色的？
        if(row < BOARD_LEN && col < BOARD_LEN)
            return curr_position[Posof(row,col)];
        return INVALID;
    }

    int PrintBoard();       // 打印当前盘面
    int Move(int pos);      // 在pos位置上落子（一般用于PVP）
    int Move(One_move move_obj);    // 用一个落子集里的方式落子（用于AI或者随机落子时）
    int RandMove();         // 随机落子

    void TimeOut(){ game_status = -curr_round; }
    // 调用超时函数会让对手胜利
private:
    int curr_round;     // 执子状态：黑方还是白方？

    // state of the curr_position
    // BLACK / WHITE / PLAYING / DRAW
    //  黑胜  / 白胜   / 对局中   / 平局
    // BLACK or WHITE means the game is over and the winner is BLACK or WHITE
    int game_status;

    // state of each position
    // BLACK / WHITE / EMPTY
    // Position 局面,棋盘落子状态，每个格子对应三种状态中一种
    int curr_position[64] = {};

    // update the moves_set
    void UpdateMovesSet();
};

#endif
