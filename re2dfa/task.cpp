#include <iostream>
#include "api.hpp"
#include <vector>
#include <string>
#include <algorithm>
#include <queue>
#include <set>


using namespace std;
vector<char> enumerate(string reg_exp) {
    vector<char> ans;
    for (auto& c : reg_exp) {
        if (c == ')' || c == '(' || c == '*' || c == '|')
            continue;
        else
            ans.push_back(c);
    }
    return ans;
}

int get_priority(char op) {
    if (op == '*')
        return 3;
    if (op == '.')
        return 2;
    if (op == '|')
        return 1;
    return -1;
}

vector<string> get_poliz(string reg_expr) {
    vector<char> stack;
    vector<string> poliz;
    bool may_concat = false, may_epsilon = true;
    int n = 1;
    for (int i = 0; i < reg_expr.size(); i++) {
        if (reg_expr[i] == '*' || reg_expr[i] == '|') {
            if (may_epsilon && reg_expr[i] == '|')
                poliz.push_back("epsilon");
            while (!stack.empty() && get_priority(stack.back()) >= get_priority(reg_expr[i])) {
                poliz.push_back(string(1, stack.back()));
                stack.pop_back();
            }
            stack.push_back(reg_expr[i]);
            may_concat = true ? reg_expr[i] == '*' : false;
            may_epsilon = true ? reg_expr[i] == '|' : false;
        }
        else if (reg_expr[i] == '(') {
            if (may_concat) {
                while (!stack.empty() && get_priority(stack.back()) >= get_priority('.')) {
                    poliz.push_back(string(1, stack.back()));
                    stack.pop_back();
                }
                stack.push_back('.');
            }
            stack.push_back(reg_expr[i]);
            may_concat = false;
            may_epsilon = true;
        } else if (reg_expr[i] == ')') {
            if (may_epsilon) {
                poliz.push_back("epsilon");
            }
            while (stack.back() != '(') {
                poliz.push_back(string(1, stack.back()));
                stack.pop_back();
            }
            stack.pop_back();
            may_concat = true;
            may_epsilon = false;
        } else {
            if (may_concat) {
                while (!stack.empty() && get_priority(stack.back()) >= get_priority('.')) {
                    poliz.push_back(string(1, stack.back()));
                    stack.pop_back();
                }
                stack.push_back('.');
            }
            poliz.push_back(to_string(n++));
            may_concat = true;
            may_epsilon = false;
        }
    }
    while (!stack.empty()) {
        poliz.push_back(string(1, stack.back()));
        stack.pop_back();
    }
    return poliz;
}

class Position {
public:
    Position(string value = "", bool type = false,
                bool is_op = false, set<string> first = {},
                set<string> last = {}) : value(value), nullable(type),
                                        is_op(is_op), first(first), last(last) {}
    string value;
    bool nullable, is_op;
    set<string> first, last;
};

class Tree {
public:
    Tree(string value, Tree* left = nullptr,
        Tree* right = nullptr, bool nullable = false,
        bool is_op = false, set<string> first = {},
        set<string> last = {}) : pos(Position(value, nullable, is_op, first, last)), left(left), right(right) {}
    Position pos;
    Tree *left, *right;
};

Tree* make_tree(vector<string>& poliz, vector<set<string>>& follow_pos) {
    vector<Tree*> stack;
    Tree *tree = nullptr;
    for (auto &e : poliz) {
        if (e == "*" || e == "|" || e == ".") {
            if (e == ".") {
                Tree *rhs = stack.back();
                stack.pop_back();
                Tree *lhs = stack.back();
                stack.pop_back();
                for (auto &i : lhs->pos.last) {
                    follow_pos[stoi(i)].insert(rhs->pos.first.begin(), rhs->pos.first.end());
                }
                bool lhs_type = lhs->pos.nullable, rhs_type = rhs->pos.nullable;
                set<string> first, last;
                first = lhs->pos.first;
                last = rhs->pos.last;
                if (lhs_type) {
                    first.insert(rhs->pos.first.begin(), rhs->pos.first.end());
                }
                if (rhs_type) {
                    last.insert(lhs->pos.last.begin(), lhs->pos.last.end());
                }
                stack.push_back(new Tree(e, lhs, rhs, lhs_type && rhs_type, true, first, last));
            } else if (e == "*") {
                Tree *lhs = stack.back();
                for (auto &i : lhs->pos.last)
                    follow_pos[stoi(i)].insert(lhs->pos.first.begin(), lhs->pos.first.end());
                stack.pop_back();
                stack.push_back(new Tree(e, lhs, nullptr, true, true, lhs->pos.first, lhs->pos.last));
            } else if (e == "|") {
                Tree *rhs = stack.back();
                stack.pop_back();
                Tree *lhs = stack.back();
                stack.pop_back();
                bool lhs_type = lhs->pos.nullable, rhs_type = rhs->pos.nullable;
                set<string> first, last;
                first = lhs->pos.first;
                last = lhs->pos.last;
                first.insert(rhs->pos.first.begin(), rhs->pos.first.end());
                last.insert(rhs->pos.last.begin(), rhs->pos.last.end());
                stack.push_back(new Tree(e, lhs, rhs, lhs_type || rhs_type, true, first, last));
            }
        } else {
            if (e == "epsilon")
                stack.push_back(new Tree(e, nullptr, nullptr, true, false, {}, {}));
            else
                stack.push_back(new Tree(e, nullptr, nullptr, false, false, {e}, {e}));
        }
    }
    tree = stack.back();
    return tree;
}

void print_tree(Tree* tree) {
    if (!tree)
        return;
    cout << "(" << tree->pos.value << " " << tree->pos.nullable << ") -> " << "first pos: ";
    for (auto &i : (tree->pos.first))
        cout << i << " ";
    cout << "last pos: ";
    for (auto &i : (tree->pos.last))
        cout << i << " ";
    cout << endl;
    print_tree(tree->left);
    print_tree(tree->right);
}

string get_name(set<string>& state) {
    string name = "";
    for (auto& i : state)
        name += i + " ";
    return name;
}

DFA re2dfa(const std::string &s) {
    if (s.size() == 0) {
        DFA res = DFA(Alphabet("a"));
        res.create_state("Start", true);
        res.set_initial("Start");
        return res;
    }
	DFA res = DFA(Alphabet(s));
	vector<char> n = enumerate("(" + s + ")" + "#");
	queue<set<string>> q;
	vector<set<string>> follow_pos(n.size() + 1);
    auto poliz = get_poliz("(" + s + ")" + "#");
	Tree *tree = make_tree(poliz, follow_pos);
    set<string> checked;
	set<string> start = tree->pos.first;
    res.create_state(get_name(start), start.count(to_string(n.size())));
    res.set_initial(get_name(start));
    q.push(start);
    while (!q.empty()) {
        set<string> state = q.front();
        q.pop();
        string from = get_name(state);
        checked.insert(from);
        for (auto& e : res.get_alphabet()) {
            set<string> new_state;
            for (auto& i : state) {
                int idx = stoi(i);
                if (n[idx - 1] == e)
                    new_state.insert(follow_pos[idx].begin(), follow_pos[idx].end());
            }
            if (!new_state.empty()) {
                string to = get_name(new_state);
                res.create_state(to, new_state.count(to_string(n.size())));
                res.set_trans(from, e, to);
                if (!checked.count(to))
                    q.push(new_state);
            }
        }
    }
	return res;
}

