#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

#define NULL_NODE -1

using namespace std;

class Node
{
private:
	template<typename T>
	static bool Remove(vector<T>& v, const T r)
	{
		if (v.size() <= 0) return false;
		bool isFound = false;
		typedef typename vector<T>::iterator iter_v;
		iter_v iter = v.begin();
		for (; iter != v.end(); ++iter)
		{
			if (r == *iter) { isFound = true; break; }
		}
		if (true == isFound) { v.erase(iter); }
		return isFound;
	}

	template<typename T>
	static bool AddNotOverlap(vector<T>& v, const T t)
	{
		typedef typename vector<T>::iterator iter_v;

		iter_v iter = std::find(v.begin(), v.end(), t);
		if (v.end() == iter)
		{
			v.push_back(t);
			return true;
		}

		return false;
	}

	template<typename T>
	static bool Contains(vector<T>& v, const T t)
	{
		typedef typename vector<T>::iterator iter_v;

		iter_v iter = std::find(v.begin(), v.end(), t);
		return iter != v.end();
	}

	static Node* Root;
	static vector<int> Goals;
	static vector<int> NodeAdjMultiGoals;
	static vector<Node*> Nodes;
	static int CntNode;
	static int CntTest;

public:
	vector<int> _adjs;
	vector<int> _adjsGoal;

	int _idx = NULL_NODE;
	int _nodeToGoal = NULL_NODE;
	bool _isGoal = false;
	bool _isActive = true;

private:
	static Node* GetNode(int node)
	{
		return Nodes[node];
	}

public:
	static void Init(const int N)
	{
		Nodes.reserve(N);

		for (int i = 0; i < N; ++i)
		{
			Nodes.push_back(new Node(i));
		}
	}

	static void ClearAll()
	{
		if (0 == CntNode) return;

		for (int i = 0; i < CntNode; ++i)
		{
			Node* n = Nodes[i];
			delete n;
		}

		Nodes.clear();
		Goals.clear();
	}

	static void AddLink(const int nodeA, const int nodeB)
	{
		Node* node = GetNode(nodeA);
		AddNotOverlap(node->_adjs, nodeB);

		node = GetNode(nodeB);
		AddNotOverlap(node->_adjs, nodeA);
	}

	static void RemoveLink(const int nodeA, const int nodeB)
	{
		Node* node = GetNode(nodeA);
		node->RemoveAdj(nodeB);
		Node::Remove(NodeAdjMultiGoals, nodeA);

		node = GetNode(nodeB);
		node->RemoveAdj(nodeA);
		Node::Remove(NodeAdjMultiGoals, nodeB);
	}

	static void SetAsGoal(int g)
	{
		Node* goal = GetNode(g);
		goal->SetGoal();
	}

	static void SetMultiGoals()
	{
		NodeAdjMultiGoals.clear();
		for (int i = 0; i < CntNode; ++i)
		{
			Node* n = Nodes[i];

			if (n->_adjsGoal.size() >= 2)
			{
				AddNotOverlap(NodeAdjMultiGoals, n->_idx);
			}
		}
	}

	struct Dist
	{
		Node* _node;
		int _dist;
		int _cntAdjGoal;

		Dist(Node* node, int dist, int cntAdjGoal)
		{
			_node = node; _dist = dist; _cntAdjGoal = cntAdjGoal;
		}
	};

	static bool CompareDegree(Dist& a, Dist& b)
	{
		if (a._cntAdjGoal == NULL_NODE && b._cntAdjGoal == NULL_NODE)
		{
			return a._dist < b._dist;
		}
		else
		{
			int priorityA = a._dist - a._cntAdjGoal;
			int priorityB = b._dist - b._cntAdjGoal;
			if (priorityA == priorityB)
			{
				return a._dist > b._dist;
			}
			return priorityA < priorityB;
		}
	}

	struct NNode
	{
		NNode* _root = nullptr;
		NNode* _parent = nullptr;
		Node* _node = nullptr;
		vector<NNode*> childs;
		bool _isRoot = false;

		void AddChild(NNode* childNode)
		{
			childs.push_back(childNode);
		}

		NNode(Node* node, NNode* parent)
		{
			_node = node; _parent = parent; _root = parent->_root;
		}

		NNode(Node* node, bool isRoot = false)
		{
			_node = node; if (isRoot == true)
			{
				_root = this;
				_isRoot = true;
			}
		}
	};

	static void GetPathNearstBfsRecurse(vector<NNode>& froms, int dist, Node* find, vector<int>& path_out, int& dist_out)
	{
		dist++;
		vector<NNode> newFroms;
		Node* found = nullptr;
		NNode* foundNnode = nullptr;

		for (int i = 0; i < froms.size(); ++i)
		{
			NNode* nnode = &froms[i];
			for (int j = 0; j < nnode->_node->_adjs.size(); ++j)
			{
				Node* node = GetNode(nnode->_node->_adjs[j]);
				if (node->_isActive == false) continue;
				if ((find == nullptr && node->_isGoal == true) || (find != nullptr && node == find))
				{
					//cerr << ">>> GetPathNearstBfsRecurse _isGoal :" << node->_idx << "/dist:" << dist << endl;
					found = node; foundNnode = nnode;
					break;
				}
				newFroms.push_back(NNode(node, nnode));
			}
			if (found != nullptr) break;
		}

		if (found != nullptr)
		{
			path_out.insert(path_out.begin(), found->_idx);
			NNode* iter = foundNnode;
			while (true)
			{
				path_out.insert(path_out.begin(), iter->_node->_idx);
				if (iter->_isRoot) break;
				iter = iter->_parent;
			}
			return;
		}

		GetPathNearstBfsRecurse(newFroms, dist, find, path_out, dist_out);
	}

	static void GetAgentToGoal(int nodeAgent, vector<int>& wayToGoal_out)
	{
		vector<int> pathToGoal;
		int dist = 0, dist_out = 9999999;
		Node* agent = GetNode(nodeAgent);
		vector<NNode> froms{ NNode(agent, true) };
		GetPathNearstBfsRecurse(froms, dist, nullptr, pathToGoal, dist_out);

		cerr << ">> pathToGoal" << pathToGoal.size() << " NodeAdjMultiGoals" << NodeAdjMultiGoals.size() << endl;

		if (3 == pathToGoal.size() && 1 <= NodeAdjMultiGoals.size())
		{
			vector<Dist> dists;
			for (int i = 0; i < NodeAdjMultiGoals.size(); ++i)
			{
				int cntAdjGoal = 0;
				Node* nodeMulti = GetNode(NodeAdjMultiGoals[i]);
				int dist = GetDistAgent(GetNode(pathToGoal[1]), nodeMulti, cntAdjGoal);
				cerr << ">>> add dist:" << dist << "/cntAdjGoal:" << cntAdjGoal << "/priority:" << (dist - cntAdjGoal) << "/nodeAgent:" << nodeAgent << "/target:" << nodeMulti->_idx << endl;
				dists.push_back(Dist(nodeMulti, dist, cntAdjGoal));
			}

			Node* resolve = GetNode(NodeAdjMultiGoals[0]);
			if (dists.size() >= 2)
			{
				std::sort(dists.begin(), dists.end(), CompareDegree);
				resolve = dists[0]._node;
			}

			Node* check = GetNode(pathToGoal[1]);
			if (1 >= check->_adjsGoal.size())
			{
				AddNotOverlap(wayToGoal_out, resolve->_idx);
				AddNotOverlap(wayToGoal_out, resolve->_adjsGoal[0]);
				//cerr << ">>>>>>> MultiGoal resolve : " << resolve->_idx << "/" << resolve->_adjsGoal[0] << endl;
				return;
			}
		}

		wayToGoal_out = pathToGoal;
		cerr << ">> don GetAgentToGoal " << pathToGoal.size() << endl;
	}

	static int GetDistAgent(Node* agent, Node* to, int& cntAdjGoal_out)
	{
		int dist = 0, distResult = NULL_NODE;
		vector<int> pathResult, path;
		vector<NNode> froms{ NNode(agent, true) };
		GetPathNearstBfsRecurse(froms, dist, to, pathResult, distResult);

		int cntAdjGoal = 0;
		if (pathResult.size() >= 1)
		{
			for (int i = 0; i < pathResult.size(); ++i)
			{
				Node* iter = GetNode(pathResult[i]);
				if (iter->IsAdjsGoalAny() == true) { cntAdjGoal++; }
			}

			cntAdjGoal_out = cntAdjGoal;
		}

		return pathResult.size();
	}

public:
	bool IsAdjsGoalAny() { return _adjsGoal.size() >= 1; }

	int GetCntGoalAdj()
	{
		int countGoal = 0;
		for (int i = 0; i < _adjs.size(); ++i)
		{
			Node* adj = Nodes[_adjs[i]];
			if (adj->_isGoal == true && adj->_isActive) ++countGoal;
		}
		return 2 <= countGoal;
	}

	bool AddAdjGoal(int idxGoal)
	{
		if (_isGoal) return false;
		AddNotOverlap(_adjsGoal, idxGoal);
		return true;
	}

	void SetGoal()
	{
		_isGoal = true;
		AddNotOverlap(Goals, _idx);
		for (int i = 0; i < _adjs.size(); ++i) GetNode(_adjs[i])->AddAdjGoal(_idx);
	}

private:
	void RemoveAdj(const int a)
	{
		Remove(_adjs, a); Remove(_adjsGoal, a);
		vector<int>::iterator found = std::find_if(_adjs.begin(), _adjs.end(), [](const int iter) { return GetNode(iter)->_isActive; });
		if (found == _adjs.end())
			_isActive = false;
	}

	Node(int node)
	{
		_idx = node;
		CntNode++;
	}

	~Node()
	{
		CntNode--;
	}
};

Node* Node::Root = nullptr;
vector<Node*> Node::Nodes;
vector<int> Node::Goals;
vector<int> Node::NodeAdjMultiGoals;
int Node::CntNode = 0;
int Node::CntTest = 0;

int main()
{
	int N; // the total number of nodes in the level, including the gateways
	int L; // the number of links
	int E; // the number of exit gateways
	cin >> N >> L >> E; cin.ignore();

	Node::Init(N);

	cerr << std::to_string(N) << " " << std::to_string(L) << " " << std::to_string(E) << endl;

	for (int i = 0; i < L; i++) {
		int N1;
		int N2;
		cin >> N1 >> N2; cin.ignore();
		Node::AddLink(N1, N2);
	}

	for (int i = 0; i < E; i++) {
		int EI; // gateway node
		cin >> EI; cin.ignore();
		Node::SetAsGoal(EI);
	}

	Node::SetMultiGoals();

	vector<int> pathToGoal;

	while (1)
	{
		int SI;
		cin >> SI; cin.ignore();

		pathToGoal.clear();
		Node::GetAgentToGoal(SI, pathToGoal);
		if (pathToGoal.size() >= 2)
		{
			int parent = pathToGoal[pathToGoal.size() - 2];
			int child = pathToGoal[pathToGoal.size() - 1];

			//cerr << "pathToGoal: " << parent << " " << child << endl;
			cout << parent << " " << child << endl;

			cerr << ">> RemoveLink" << endl;
			Node::RemoveLink(parent, child);
			cerr << ">> Done RemoveLink" << endl;
		}
	}

	Node::ClearAll();
}