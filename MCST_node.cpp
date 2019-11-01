#include"MCST_node.h"

MCST_node::MCST_node(Reversi_game *othello)
        :game(othello),parent(nullptr),
         black_n(0),white_n(0),v_n(0),a_n(0.0),
         child(std::vector<MCST_node*>(othello->moves_set.size(), (MCST_node*) nullptr)){ }

MCST_node::~MCST_node() {
    delete game; game = nullptr;
}

MCST_node* MCST_node::SearchAndPlay() {
    //get the time limit
    const double time_lim = 58; // = EstimatTimeLimit(); // 修改为固定数值（<= 58 s）以获得更多的搜索
    std::cout << "Time estimation of current search: " << time_lim <<"s"<< std::endl;
    time_t s_time, t_time;
    time(&s_time);
    int i = 0;
    int search_end;
    // MCTSing until search ends or reach the time limit
    while(1){
        search_end = SearchOnce();
        if (search_end) { std::cout << "Search finished." << std::endl; break; }
        i++;
        time(&t_time);
        if (difftime(t_time, s_time) > time_lim) break;
    }
    // choose the best child
    int n = -1;
    double max_ucb = -2;
    double t_ucb;
    for (int i = 0; i < child.size(); i++) {
        if (!child.at(i)) continue;
        t_ucb = (double)child.at(i)->a_n / child.at(i)->v_n;
        if (t_ucb > max_ucb){
            max_ucb = t_ucb;
            n = i;
        }
    }
    if (n < 0) {
        std::cout << "Search Not Enough!" << std::endl;
        return this;
    }
    double diff_t = difftime(t_time, s_time);
    if (diff_t > 1e-4 && diff_t < 60)
        total_time += diff_t;
    std::cout << "Piece located at " << Reversi_game::Xof(game->moves_set.at(n).move_pos) << " " << Reversi_game::Yof(game->moves_set.at(n).move_pos) << " "
              << "\tSearched " << i << " steps with " << diff_t <<" s, total cost: "<< total_time <<" s"<< std::endl;
    // Move out
    return Play(n);
}

void MCST_node::BackPropagation(double val) {
    for (MCST_node *t_node = this; t_node; t_node = t_node->parent) {
        t_node->v_n++; t_node->a_n += val;
    }
}

MCST_node* MCST_node::Play(int n) {
    MCST_node *n_node = child.at(n);
    if (!n_node) {
        Reversi_game *t_othello = new Reversi_game(*game);
        t_othello->Move(t_othello->moves_set.at(n));
        n_node = new MCST_node(t_othello);
    }
    // separate the child and the root
    // then free the tree of the root
    n_node->parent = nullptr;
    child[n] = nullptr;
    FreeTree();
    return n_node;
}

MCST_node* MCST_node::Play(int x, int y) {
    int n = -1;
    for (int i = 0; i < game->moves_set.size(); i++) {
        if (Reversi_game::Posof(x, y) == game->moves_set.at(i).move_pos) { n = i; break; }
    }
    if (n >= 0) return Play(n);
    else { std::cout << "Invalid, try another move." << std::endl; return this; }
}

void MCST_node::FreeTree() {
    for ( auto &it : child ) if(it){
            it->FreeTree(); // 递归调用每个子节点的FreeTree
            delete it; it = nullptr;
        }
}

void MCST_node::PrintTree(int level) {
    for (int i = 0; i < level; i++) std::cout << "  ";
    std::cout << "|| " << a_n << " / " << v_n << std::endl;
    for ( const auto &it : child ) if(it) it->PrintTree(level + 1);
}
void MCST_node::PrintTree(int level, int target_level) {
    for (int i = 0; i < level; i++) std::cout << "  ";
    std::cout << "|| " << a_n << " / " << v_n << std::endl;
    if (level >= target_level) return;
    for (int i = 0; i < child.size(); i++) {
        for (int i = 0; i < level + 1; i++) std::cout << "  ";
        std::cout << "x= " << Reversi_game::Xof(game->moves_set.at(i).move_pos)<< " y= " << Reversi_game::Yof(game->moves_set[i].move_pos) << std::endl;
        if (child.at(i)) child.at(i)->PrintTree(level + 1, target_level);
    }
}

int MCST_node::TreePolicy() {
    static const double r = 2;
    // if there is no child, return ucb_i=-1
    int ucb_i = -1;
    double ucb_val = -2;
    for (int i = 0; i < child.size(); i++) {
        MCST_node *tnode = child.at(i);
        if (!tnode) return i;
        if (!tnode->v_n) return i;
        // use the fomula of UCT
        double t_val = (double)tnode->a_n / tnode->v_n + r * sqrt(log((double)v_n / tnode->v_n));
        if (t_val > ucb_val) { ucb_val = t_val; ucb_i = i; }
    }
    return ucb_i;
}

int MCST_node::DefaultPolicy() {
    // randomly roll out
    Reversi_game t_othello(*game);
    while (t_othello.WhosRound() == Reversi_game::PLAYING) {
        // 对应刚开局的时候？
        // 不是 t_othello.GameStatus() == Othello_game::PLAYING
        t_othello.RandMove();
    }
    return t_othello.WhosRound();
}

int MCST_node::SearchOnce() {
    MCST_node *t_node = nullptr;
    MCST_node *n_node = this;
    int n;
    while (n_node) {    // recursively get the child to be expanded
        t_node = n_node;
        n = t_node->TreePolicy();
        if (n == -1) return 1;
        else n_node = t_node->child.at(n);
    }
    // expand the node
    Reversi_game *t_othello = new Reversi_game(*(t_node->game));
    t_othello->Move(t_othello->moves_set.at(n));
    n_node = new MCST_node(t_othello);
    n_node->parent = t_node;
    t_node->child[n] = n_node;
    // simulation and back propagaion
    int simu_v = t_node->DefaultPolicy();
    if (simu_v == game->WhosRound()) n_node->BackPropagation(1);
    else if(simu_v == -(game->WhosRound())) n_node->BackPropagation(0);
    else n_node->BackPropagation(0.5);
    return 0;
}
