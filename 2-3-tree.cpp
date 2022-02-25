#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;

template<class ValueType>
class Set {
public:
	static bool is_equal(ValueType a, ValueType b) {
		return !(a < b || b < a);
	}

	class Node {
	public:
		static void insert(Node* node, ValueType x, Node*& old_root) {
			Node tmp(x);
			Node* new_node = new Node(tmp);
			update(new_node);
			if (!node) {
				old_root = new_node;
				update(old_root);
				return;
			}
			node = lower_bound(node, x);
			if (is_equal(node->value, x)) {
				delete new_node;
				return;
			}
			if (!node->parent) {
				if (new_node->value < node->value) {
					swap(node, new_node);
				}
				Node tmp(new_node->value);
				tmp.parent = nullptr, tmp.kids = {node, new_node};
				Node* new_root = new Node(tmp);
				node->parent = new_node->parent = new_root;
				old_root = new_root;
			} else {
				add_kid(new_node, node->parent);
				update_all_parents(node);
				split(node->parent, old_root);
			}
			update_all_parents(node);
		}

		static Node* lower_bound(Node* node, ValueType x) {
			if (!node) {
				return node;
			}
			while (!node->kids.empty()) {
				if (x < node->kids[0]->value || is_equal(x, node->kids[0]->value)) {
					node = node->kids[0];
				} else if (node->kids.size() == 2 || x < node->kids[1]->value || is_equal(x, node->kids[1]->value)) {
					node = node->kids[1];
				} else {
					node = node->kids[2];
				}
			}
			return node;
		}

		static void erase(Node* node, Node*& old_root) {
			if (!node->parent) {
				old_root = nullptr;
				delete node;
				return;
			}
			if (node->parent->kids.size() == 3) {
				node->parent->kids.erase(remove(node->parent->kids.begin(), node->parent->kids.end(), node), node->parent->kids.end());
				update_all_parents(node);
				delete node;
				return;
			}
			Node* grandparent = node->parent->parent;
			Node* brother = node->parent->kids[0];
			if (brother == node) {
				brother = node->parent->kids[1];
			}
			if (!grandparent) {
				old_root = brother;
				delete brother->parent;
				brother->parent = nullptr;
				delete node;
				return;
			}
			node->parent->kids.erase(remove(node->parent->kids.begin(), node->parent->kids.end(), node), node->parent->kids.end());
			bool is_uncle_left;
			Node* uncle = get_neighbour_uncle(node, is_uncle_left);
			if (uncle->kids.size() == 2) {
				add_kid(brother, uncle);
				update(node->parent);
				erase(node->parent, old_root);
			} else {
				if (is_uncle_left) {
					add_kid(uncle->kids.back(), node->parent);
					uncle->kids.pop_back();
				} else {
					add_kid(uncle->kids[0], node->parent);
					uncle->kids.erase(uncle->kids.begin());
				}
				update(uncle);
				update_all_parents(node);
			}
			delete node;
		}

		Node* parent = nullptr;
		vector<Node*> kids;
		ValueType value;
		size_t size;

	private:
		explicit Node(const ValueType& other) {
			parent = 0;
			value = other;
			size = 1;
		}

		static void update_size(Node* node) {
			if (!node) {
				return;
			}
			if (!(node->kids).empty()) {
				node->size = 0;
				for (auto& now : node->kids) {
					node->size += now->size;
				}
			} else {
				node->size = 1;
			}
		}

		static void update(Node* node) {
			update_value(node);
			update_size(node);
		}

		static void update_all_parents(Node* node) {
			while (node->parent) {
				update(node->parent);
				node = node->parent;
			}
		}

		static void update_value(Node* node) {
			if (!(node->kids).empty()) {
				node->value = node->kids.back()->value;
			}
		}

		static void add_kid(Node* kid, Node* par) {
			kid->parent = par;
			auto& arr = par->kids;
			arr.push_back(kid);
			int ind = static_cast<int>(arr.size()) - 1;
			while (ind && arr[ind]->value < arr[ind - 1]->value) {
				swap(arr[ind], arr[ind - 1]);
				--ind;
			}
			update(par);
		}

		static void split(Node* node, Node*& old_root) {
			if (node->kids.size() >= 4) {
				Node tmp(node->kids[3]->value);
				tmp.parent = node->parent, tmp.kids = {node->kids[2], node->kids[3]};
				Node* brother = new Node(tmp);
				node->kids[2]->parent = node->kids[3]->parent = brother;
				node->kids.pop_back();
				node->kids.pop_back();
				node->value = node->kids[1]->value;
				update(brother);
				update(node);
				if (node->parent) {
					add_kid(brother, node->parent);
					split(node->parent, old_root);
				} else {
					Node tmp(brother->value);
					tmp.parent = nullptr, tmp.kids = {node, brother};
					Node* new_root = new Node(tmp);
					node->parent = brother->parent = new_root;
					old_root = new_root;
					update(new_root);
				}
			}
		}


		static Node* get_neighbour_uncle(Node* node, bool& is_uncle_left) {
			is_uncle_left = true;
			Node* grandparent = node->parent->parent;
			Node* uncle = grandparent->kids[0];
			if (uncle == node->parent) {
				uncle = grandparent->kids[1];
				is_uncle_left = false;
			}
			if (grandparent->kids.size() == 3 && node->parent == grandparent->kids.back()) {
				uncle = grandparent->kids[1];
				is_uncle_left = true;
			}
			return uncle;
		}
	};

	class iterator {
	public:
		const Node* node = nullptr;
		bool last = true;
		const Set* ref = nullptr;

		iterator() {
			node = nullptr;
			last = true;
			ref = nullptr;
		}
		
		iterator(const Node* to, bool flag, const Set* other) {
			node = to;
			last = flag;
			ref = other;
		}

		const iterator operator++() {
			if (!node) {
				return *this = iterator(nullptr, true, ref);
			}
			while (node->parent && node->parent->kids.back() == node) {
				node = node->parent;
			}
			if (!node->parent) {
				while (!(node->kids).empty()) {
					node = node->kids.back();
				}
				return *this = iterator(node, true, ref);
			}
			if (node->parent->kids[0] == node) {
				node = node->parent->kids[1];
			} else {
				node = node->parent->kids[2];
			}
			while (!node->kids.empty()) {
				node = node->kids[0];
			}
			return *this = iterator(node, false, ref);
		}

		iterator operator++(int) {
			const iterator result = *this;
			++(*this);
			return result;
		}

		iterator operator--() {
			if (!node) {
				return *this = iterator(nullptr, true, ref);
			}
			if (last) {
				last = false;
				return *this;
			}
			while (node->parent && node->parent->kids[0] == node) {
				node = node->parent;
			}
			if (!node->parent) {
				while (!(node->kids).empty()) {
					node = node->kids.back();
				}
				return *this = iterator(node, true, ref);
			}
			if (node->parent->kids[1] == node) {
				node = node->parent->kids[0];
			} else {
				node = node->parent->kids[1];
			}
			while (!node->kids.empty()) {
				node = node->kids.back();
			}
			return *this = iterator(node, false, ref);
		}

		const iterator operator--(int) {
			iterator result = *this;
			--(*this);
			return result;
		}

		bool operator!=(const iterator& other) {
			return !(*this == other);
		}

		bool operator==(const iterator& other) {
			return node == other.node && last == other.last && ref == other.ref;
		}

		const ValueType& operator*() const {
			return node->value;
		}

		const ValueType* operator->() const {
			return &(node->value);
		}
	};

	Set() : root(nullptr) {
	}

	template<class iter>
	Set(iter begin_, iter end_) {
		root = nullptr;
		while (begin_ != end_) {
			insert(*begin_);
			begin_++;
		}
	}

	Set(std::initializer_list<ValueType> list) {
		root = nullptr;
		for (auto& now : list) {
			insert(now);
		}
	}

	bool empty() const {
		return !root;
	}

	Set& operator=(const Set& other) {
		if (this == &other) {
			return *this;
		}
		Destructor();
		for (auto& now : other) {
			insert(now);
		}
		return *this;
	}

	Set(const Set& other) {
		root = nullptr;
		for (auto& now : other) {
			insert(now);
		}
	}

	void WalkThrough(Node* node, vector<Node*>& arr) {
		if (!node) {
			return;
		}
		for (auto& kid : node->kids) {
			WalkThrough(kid, arr);
		}
		arr.push_back(node);
	}

	void Destructor() {
		vector<Node*> arr;
		WalkThrough(root, arr);
		for (auto& now : arr) {
			delete now;
		}
		root = nullptr;
	}

	~Set() {
		Destructor();
	}

	size_t size() const {
		return (root ? root->size : 0);
	}

	void insert(const ValueType& elem) {
		Node::insert(root, elem, root);
	}

	void erase(const ValueType& elem) {
		Node* node = Node::lower_bound(root, elem);
		if (!node || !is_equal(node->value, elem)) {
			return;
		}
		Node::erase(node, root);
	}

	iterator find(ValueType x) const {
		if (!root) {
			return iterator(nullptr, true, this);
		}
		Node* node = Node::lower_bound(root, x);
		if (!is_equal(node->value, x)) {
			while (node->parent) {
				node = node->parent;
			}
			while (!(node->kids).empty()) {
				node = node->kids.back();
			}
			return iterator(node, true, this);
		}
		return iterator(node, false, this);
	}

	iterator lower_bound(ValueType x) const {
		if (!root) {
			return iterator(nullptr, true, this);
		}
		Node* node = Node::lower_bound(root, x);
		if (node->value < x) {
			return iterator(node, true, this);
		}
		return iterator(node, false, this);
	}

	iterator begin() const {
		if (!root) {
			return iterator(nullptr, true, this);
		}
		Node* cur = root;
		while (!cur->kids.empty()) {
			cur = cur->kids[0];
		}
		return iterator(cur, false, this);
	}

	iterator end() const {
		if (!root) {
			return iterator(nullptr, true, this);
		}
		Node* cur = root;
		while (!cur->kids.empty()) {
			cur = cur->kids.back();
		}
		return iterator(cur, true, this);
	}

private:
	Node* root;
};
