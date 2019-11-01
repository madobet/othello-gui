#ifndef MCST_NODE_H
#define MCST_NODE_H

#include<iostream>
#include<math.h>
#include<time.h>
#include"Reversi_game.h"

extern double total_time;

class MCST_node {   // 蒙特卡罗搜索树的节点
public:
    // 当前节点对应的黑白棋的棋局状态
    Reversi_game *game;

    MCST_node(Reversi_game *othello);
    ~MCST_node();

    //double TimeNeeded() { return EstimatTimeLimit();}
    MCST_node* SearchAndPlay();
    MCST_node* Play(int n);
    MCST_node* Play(int x, int y);
    void BackPropagation(double val);
    void FreeTree();

    // 打印出指定层次的当前搜索树，用于调试
    void PrintTree(int level, int tatrgetlevel);
    void PrintTree(int level);

private:
    MCST_node *parent;              // 父节点
    std::vector<MCST_node*> child;  // 子节点数组
    // Monte Carlo Value for the calcutation of UCT
    int v_n;
    double a_n;
    // number of chesses on the curr_position
    int black_n;
    int white_n;

    // get the search time bound
    inline double EstimatTimeLimit(){
        double base = 30;       // 预估时间基数
        double chil_fact;
        double process_fact;

        int csize = child.size();
        if(csize == 1) return 0;
        if (csize > 10) csize = 10;
        chil_fact = csize / 2.0;

        int count = black_n + white_n;
        if (count < 32) process_fact = 0;
        else process_fact = (32.0 - count) / 32.0 * 15.0;

        double res = base + chil_fact + process_fact;
        return (res > 58) ? 58 : res ;
    }
    int TreePolicy();
    int DefaultPolicy();
    int SearchOnce();
};

#endif
