#pragma once
#include <memory>
#include <list>

namespace tcl {
	template<class TypeBranch, class TypeLeaf>
	class hierarchy_list {
		template<typename TB, typename TL>
		class node_type {
		public:
			using type_branch = TB;
			using type_leaf = TL;
		public:
			TB								_brch;
			std::list<node_type<TB, TL>>	_chds;
			TL								_leaf;

			template<typename ... BCTOR_ARG>
			node_type(BCTOR_ARG&& ... ctor) : _brch(std::forward<BCTOR_ARG&&>(ctor)...) { ; }

			node_type& operator = (const node_type& n) {
				_brch = n._brch;
				_chds = n._chds;
				_leaf = n._leaf;
				return *this;
			}
		};
		using node = node_type<TypeBranch, TypeLeaf>;
		using iterator_branch = typename std::list<node>::iterator;

		class insert_iterator {
			friend class hierarchy_list;
			mutable node* _node;

			insert_iterator(node* n) : _node(n), first(&n->_brch), second(&n->_leaf) { ; }
		public:
			typename node::type_branch* first;
			typename node::type_leaf* second;

			template<typename ... BRANCH_CTOR_ARG>
			decltype(auto) emplace_branch(BRANCH_CTOR_ARG&& ... args) const {
				_node->_chds.emplace_back(args...);
				return insert_iterator(&_node->_chds.back());
			}

			template<typename ... LEAF_CTOR_ARG>
			typename node::type_leaf& emplace_leaf(LEAF_CTOR_ARG&& ... args) const {
				_node->_leaf = TypeLeaf(std::forward<LEAF_CTOR_ARG&&>(args)...);
				return _node->_leaf;
			}
		};

		class traverse_iterator {
			friend class hierarchy_list;

			mutable std::list<std::pair<iterator_branch, iterator_branch>> it_tree;
			mutable std::list<typename node::type_branch> it_path;

			traverse_iterator(node& n) : it_tree() {
				it_tree.push_front(std::make_pair(n._chds.begin(), n._chds.end()));
				it_path.push_front(n._brch);
			}

			class tuple {
				friend class traverse_iterator;
				tuple(typename node::type_branch& _branch, typename node::type_leaf& _leaf,
					std::list<typename node::type_branch>& _path) :
					branch(_branch), leaf(_leaf), path(_path) {
					;
				}
			public:
				typename node::type_branch& branch;
				typename node::type_leaf& leaf;
				std::list<typename node::type_branch>& path;
			};

		public:

			~traverse_iterator() { ; }

			decltype(auto) operator * () {
				return tuple(
					it_tree.front().first->_brch, it_tree.front().first->_leaf, it_path
				);
			}

			inline traverse_iterator& operator++() {
				if (!it_tree.front().first->_chds.empty()) {
					it_path.push_front(it_tree.front().first->_brch);
					it_tree.push_front(std::make_pair(it_tree.front().first->_chds.begin(), it_tree.front().first->_chds.end()));
					return *this;
				}
				while (!it_tree.empty()) {
					it_tree.front().first++;
					if (it_tree.front().first != it_tree.front().second) {
						break;
					}
					it_tree.pop_front();
					it_path.pop_front();
				}
				return *this;
			}

			inline bool operator != (const traverse_iterator&) {
				return !it_tree.empty() && it_tree.front().first != it_tree.front().second;
			}
		};

		mutable node	childs;
	public:

		using iiterator = insert_iterator;
		using iterator = traverse_iterator;

		hierarchy_list() { ; }
		~hierarchy_list() { ; }

		template<typename ... BRANCH_CTOR_ARG>
		decltype(auto) emplace_branch(BRANCH_CTOR_ARG&& ... args) const {
			childs._chds.emplace_back(std::forward<BRANCH_CTOR_ARG&&>(args)...);
			return iiterator(&childs._chds.back());
		}

		template<typename ... LEAF_CTOR_ARG>
		typename node::type_leaf& emplace_leaf(LEAF_CTOR_ARG&& ... args) const {
			childs._leaf = TypeLeaf(std::forward<LEAF_CTOR_ARG&&>(args)...);
			return childs._leaf;
		}

		inline bool empty() { return childs._chds.empty(); }
		
		inline void clear() { childs._chds.clear(); childs._brch = TypeBranch();  childs._leaf = TypeLeaf(); }

		inline decltype(auto) begin() { return iterator(childs); }

		inline decltype(auto) end() { return iterator(childs); }
	};
}