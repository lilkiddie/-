#include "api.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <set>

using namespace std;

map<string, map<string, string>> classes; // class_num -> (state -> transitions)
map<string, string> states; // state -> class_num
int class_num = 3;


void fill(DFA &d) {
	for (auto& cl : classes)
		for (auto& state : cl.second)
			state.second = "";
	for (auto& cl : classes) {
		for (auto& state : cl.second) {
			for (auto& c : d.get_alphabet()) {
				if (d.has_trans(state.first, c)) {
					string trans = d.get_trans(state.first, c);
					state.second += states[trans] + " ";
				} else {
					state.second += (states["dead"]) + " ";
				}
			}
		}
	}
}


bool split_class(string n) {
	map<string, string>& m = classes[n]; // work with n_th class
	map<string, set<string>> new_m;
	bool changed = false;
	// iteration over all states
	for (auto i : m) {
		if (!new_m.count(i.second))
			new_m[i.second] = {};
		new_m[i.second].insert(i.first);
	}
	classes.erase(n);
	if (new_m.size() > 1)
		changed = true;
	for (auto it = new_m.begin(); it != new_m.end(); it++) {
		if (it == new_m.begin()) {
			for (auto& state : it->second)
				classes[n][state] = "";
		} else {
			string new_n = to_string(class_num++);
			for (auto& state : it->second) {
				classes[new_n][state] = "";
				states[state] = new_n;
			}
		}
	}
	return changed;
}


bool is_dead(map<string, string>& s) {
	for (auto& [key, value] : s) {
		if (key == "dead")
			return true;
	}
	return false;
}

bool is_initial(map<string, string>& s, DFA& d) {
	for (auto& [key, value] : s) {
		if (d.is_initial(key))
			return true;
	}
	return false;
}

bool isfinal(map<string, string>& s, DFA& d) {
	for (auto& [key, value] : s) {
		if (d.is_final(key))
			return true;
	}
	return false;
}

void dfs(DFA& d, set<string>& visited, string v) {
	visited.insert(v);
	for (auto& c : d.get_alphabet()) {
		if (d.has_trans(v, c)) {
			string state = d.get_trans(v, c);
			if (!visited.count(state))
				dfs(d, visited, state);
		}
	}
}

DFA dfa_minim(DFA &d) {
	classes["1"]["dead"] = {};
	states["dead"] = "1";
	bool changed = true;
	for (auto& state : d.get_states()) {
		if (d.is_final(state)) {
			states[state] = "2";
			classes["2"][state] = "";
		}
		else {
			states[state] = "1";
			classes["1"][state] = "";
		}
	}

	fill(d);
	while (changed) {
		changed = false;
		for (int i = 1; i < class_num; i++) {
			changed = changed || split_class(to_string(i));
		}
		fill(d);
	}

	//for (auto& cl : classes) {
	//	cout << '\t' << cl.first << endl;
	//	for (auto& state : cl.second) {
	//		cout << state.first << ": ";
	//		for (auto& i : state.second)
	//			cout << i << " ";
	//		cout << endl;
	//	}
	//}
	DFA new_d(d.get_alphabet());
	string dead = "";
	for (auto& _class : classes) {
		if (is_dead(_class.second)) {
			dead = _class.first;
			continue;
		}
		new_d.create_state(_class.first);
	}

	for (auto& state : new_d.get_states()) {
		if (is_initial(classes[state], d))
			new_d.set_initial(state);
		if (isfinal(classes[state], d))
			new_d.make_final(state);
		int idx = 0;
		stringstream parse(classes[state].begin()->second);
		string word;
		for (auto& c : new_d.get_alphabet()) {
			parse >> word;
			new_d.set_trans(state, c, word);
		}
	}
	string start = "";
	for (auto& state : new_d.get_states())
		if (new_d.is_initial(state))
			start = state;
	set<string> visited;
	cout << start << endl;
	dfs(new_d, visited, start);
	for (auto& state : new_d.get_states()) {
		if (!visited.count(state))
			new_d.delete_state(state);
	}
	return new_d;
}
