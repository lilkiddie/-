#pragma once
#include "api.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <set>
#include <map>

using namespace std;

map<string, int> state_to_int;
map<int, string> int_to_state;
vector<vector<vector<string>>> graph;

void unite() {
	for (auto& i : graph) {
		for (auto& j : i) {
			if (j.size() > 1) {
				string ans = "";
				for (int i = 0; i < j.size(); i++) {
					if (i == 0)
						ans = j[i];
					else
						ans += "|" + j[i];
				}
				j.clear();
				j.push_back(ans);
			}
		}
	}
}


vector<int> get_ins(int to) {
	vector<int> ans;
	for (int i = 0; i < graph.size(); i++) {
		if (!graph[i][to].empty()) {
			ans.push_back(i);
		}
	}
	return ans;
}


vector<int> get_outs(int from) {
	vector<int> ans;
	for (int i = 0; i < graph.size(); i++) {
		if (!graph[from][i].empty())
			ans.push_back(i);
	}
	return ans;
}

void print() {
	for (auto& i : graph) {
		for (auto& j : i) {
			if (j.empty()) {
				cout << "-";
			}
			else {
				for (auto& k : j)
					cout << k << " ";
			}
			cout << '\t';
		}
		cout << endl;
	}
	cout << endl;
}

string dfa2re(DFA& d) {
	int state_num = 0;
	set<int> final_states;
	int initial = -1;
	for (auto& state : d.get_states()) {
		if (d.is_initial(state))
			initial = state_num;
		if (d.is_final(state))
			final_states.insert(state_num);
		state_to_int[state] = state_num;
		int_to_state[state_num] = state;
		state_num++;
	}
	
	state_to_int["final"] = state_num;
	int_to_state[state_num] = "final";

	int size = state_num + 1;

	for (auto& [i, from] : int_to_state) {
		graph.push_back(vector<vector<string>>(size));
		for (auto& c : d.get_alphabet()) {
			if (d.has_trans(from, c)) {
				string to = d.get_trans(from, c);
				int j = state_to_int[to];
				graph[i][j].push_back(string(1, c));
			}
		}
	}

	for (auto& state : final_states) {
		graph[state][size - 1].push_back("");
	}


	for (auto it = int_to_state.begin(); it != int_to_state.end(); it++) {
		if (it->first == initial || it->second == "final")
			continue;
		unite();
		int query = it->first;
		vector<int> ins = get_ins(query), outs = get_outs(query);
		string loop = "";
		if (!graph[query][query].empty())
			loop = *graph[query][query].begin();
		for (int i = 0; i < ins.size(); i++) {
			int from = ins[i];
			string in = graph[from][query][0];
			if (in != "")
				in = "(" + in + ")";
			else
				in = "";
			for (int j = 0; j < outs.size(); j++) {
				int to = outs[j];
				string out = graph[query][to][0];
				if (out != "")
					out = "(" + out + ")";
				else
					out = "";
				if (loop != "")
					graph[from][to].push_back(in + "(" + loop + ")*" + out);
				else
					graph[from][to].push_back(in + out);
			}
		}
		for (int i = 0; i < ins.size(); i++) {
			int from = ins[i];
			graph[from][query].clear();
		}
		for (int i = 0; i < outs.size(); i++) {
			int to = outs[i];
			graph[query][to].clear();
		}
	}
	unite();
	int to = state_to_int["final"];
	if (!graph[initial][initial].empty()) {
		return "(" + graph[initial][initial][0] + ")*" + "(" + graph[initial][to][0] + ")";
	}

	return graph[initial][to][0];
}