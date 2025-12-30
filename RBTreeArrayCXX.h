/*
 * Continuous Memory Red-Black Tree (RBTreeArray)
 * ==============================================
 * 
 * Overview:
 * ---------
 * A high-performance, contiguous memory implementation of a Red-Black Tree.
 * All nodes are stored in a single contiguous memory block, enabling efficient
 * serialization to/from files or shared memory. This design provides O(log n)
 * time complexity for insert, search, and delete operations, with performance
 * significantly exceeding that of std::map in many scenarios.
 * 
 * Key Features:
 * -------------
 * - Contiguous memory layout for serialization and fast traversal
 * - Efficient serialization to/from files or shared memory
 * - Faster than std::map in insert, search, and delete operations
 * - Support for user-defined types and STL containers as key/value types
 * - In-place construction/destruction via placement new
 * - Memory shrinkage and resize operations
 * - Two iterator types: unordered (fast) and ordered (key-sorted)
 * - Conditional deletion with configurable predicates
 * 
 * Speed Test: use g++ -O2
 * -----------------------
 * RBTreeArray16:
 *   Insert: RBTreeArray16<unsigned,unsigned>: 693 , std::map<unsigned,unsigned>: 1029 milliseconds
 *   Search: RBTreeArray16<unsigned,unsigned>: 651 , std::map<unsigned,unsigned>: 924 milliseconds
 *   Loop Through: RBTreeArray16<unsigned,unsigned>: 5 , std::map<unsigned,unsigned>: 355 milliseconds
 *   Delete: RBTreeArray16<unsigned,unsigned>: 795 , std::map<unsigned,unsigned>: 993 milliseconds
 *   Conditional Delete 1/3 keys: RBTreeArray16<unsigned,unsigned>: 351 , std::map<unsigned,unsigned>: 350 milliseconds
 *   Conditional Delete 1/10 keys: RBTreeArray16<unsigned,unsigned>: 114 , std::map<unsigned,unsigned>: 482 milliseconds
 * RBTreeArray32:
 *   Insert: RBTreeArray32<unsigned,unsigned>: 611 , std::map<unsigned,unsigned>: 875 milliseconds
 *   Search: RBTreeArray32<unsigned,unsigned>: 755 , std::map<unsigned,unsigned>: 1257 milliseconds
 *   Loop Through: RBTreeArray32<unsigned,unsigned>: 4 , std::map<unsigned,unsigned>: 236 milliseconds
 *   Delete: RBTreeArray32<unsigned,unsigned>: 632 , std::map<unsigned,unsigned>: 1189 milliseconds
 *   Conditional Delete 1/3 keys: RBTreeArray32<unsigned,unsigned>: 212 , std::map<unsigned,unsigned>: 241 milliseconds
 *   Conditional Delete 1/10 keys: RBTreeArray32<unsigned,unsigned>: 97 , std::map<unsigned,unsigned>: 240 milliseconds
 * RBTreeArray64:
 *   Insert: RBTreeArray64<unsigned,unsigned>: 909 , std::map<unsigned,unsigned>: 957 milliseconds
 *   Search: RBTreeArray64<unsigned,unsigned>: 924 , std::map<unsigned,unsigned>: 1280 milliseconds
 *   Loop Through: RBTreeArray64<unsigned,unsigned>: 5 , std::map<unsigned,unsigned>: 237 milliseconds
 *   Delete: RBTreeArray64<unsigned,unsigned>: 785 , std::map<unsigned,unsigned>: 1200 milliseconds
 *   Conditional Delete 1/3 keys: RBTreeArray64<unsigned,unsigned>: 252 , std::map<unsigned,unsigned>: 225 milliseconds
 *   Conditional Delete 1/10 keys: RBTreeArray64<unsigned,unsigned>: 134 , std::map<unsigned,unsigned>: 236 milliseconds
 * 
 * Capacity Limits:
 * ----------------
 * - Three variants with different capacity limits:
 *     RBTreeArray16: up to 65535 key-value pairs
 *     RBTreeArray32: up to 4294967295 key-value pairs
 *     RBTreeArray64: up to 18446744073709551615 key-value pairs
 * 
 * Type Requirements:
 * ------------------
 * Key types must implement operator < and > for comparison
 * Both key and value types must be trivially copyable or movable
 * 
 * Example Usage:
 * --------------
 *     RBTreeArray32<std::string,std::vector<double>> tree32;
 *     RBTreeArray16<double,unsigned> tree16;
 *     RBTreeArray64<unsigned long long,std::vector<std::pair<std::string,double>>> tree64;
 * 
 *     tree32.Insert("pi",{3.14159});
 *     double value;
 *     if(tree32.Search("pi",value)){
 * 	   }
 * 	   tree32.Delete("pi");
 *     // ...
 *     // more example please see Public Interface Detail
 * 
 * Public Interface Summary:
 * -------------------------
 * Construction:
 *   - Default, sized, initializer_list, copy, and move constructors
 * 
 * Core Operations:
 *   - Insert(key, value)        // Insert or update
 *   - Delete(key)               // Remove by key
 *   - Search(key, value)        // Lookup value by key
 *   - GetMin/GetMax             // Retrieve extreme elements
 *   - GetSmallestGreaterThan / GetBiggestSmallerThan  // Neighborhood queries
 * 
 * Bulk Operations:
 *   - ConditionalDelete          // Remove all matching a predicate
 *   - ConditionalDeleteOnce      // Remove first match
 *   - Keys() / Values()          // Extract all keys/values
 *   - KeysValues()               // Extract all pairs
 * 
 * Memory Management:
 *   - MemoryShrink()            // Shrink to fit current size
 *   - ReSize(newSize)           // Resize capacity
 *   - Clear()                   // Remove all elements (keeps memory)
 *   - Data()                    // Get raw C-style pointer to underlying structure
 *   - ByteSize()                // Get total memory footprint
 * 
 * Tree Operations:
 *   - SetTree()                 // Replace with external tree (take ownership)
 *   - SetTreeWithoutDestroyMyTree()  // Replace without destroying current
 *   - Transform()               // Convert between different bit-length variants
 * 
 * Iterators:
 *   - begin() / end()           // Unordered iterators (fast traversal)
 *   - OrderedBegin() / OrderedEnd()  // Key-ordered iterators
 *   - Range-based for loop support (C++17) // Unordered, fast traversal
 * 
 * Notes:
 * ------
 * - Pointers returned by KeysPointer(), ValuesPointer(), etc. are invalidated
 *   by any structural modification (insert, delete, resize).
 * - For maximum performance, use UnorderedIterator when order is not required.
 * - Use MemoryShrink() after Clear() to release unused memory.
 * 
 * Thread Safety:
 * --------------
 * This implementation is not thread-safe. External synchronization is required
 * for concurrent access.
 * 
 * Exception Safety:
 * -----------------
 * Most operations provide strong exception guarantee. Exceptions may be thrown
 * by memory allocation failures or invalid operations (e.g., out-of-range access).
 * 
 * 
 * Public Interface Details:
 * -------------------------
 * 
 * RBTreeArray();
 *     Default constructor, creat RBTreeArray with default array size(256), 
 *     Usage example: 
 *         RBTreeArray32<std::string,std::vector<double>> tree32;
 *         RBTreeArray16<double,unsigned> tree16;
 * 
 * RBTreeArray(uint64_t size);
 *     Constructor, creat RBTreeArray with specific size
 *     Usage example: 
 *         RBTreeArray32<std::string,std::vector<double>> tree32(100000);
 *         RBTreeArray16<double,unsigned> tree16(65535);
 *     If size >= the most size that the tree allowed, it will create RBTreeArray with the most size that the tree allowed
 * 
 * RBTreeArray(std::initializer_list<std::pair<KeyType,ValueType>> initList);
 *     Constructor, creat RBTreeArray with with default array size(256) and std::initializer_list
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *     
 * RBTreeArray(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);
 *     Constructor, creat RBTreeArray by copying from another RBTreeArray
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *         RBTreeArray32<unsigned,double> tree32Copy=tree32; // tree32: {{1,2},{3,4},{5,6}}, tree32Copy: {{1,2},{3,4},{5,6}}
 * 
 * RBTreeArray(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);
 *     Constructor, creat RBTreeArray by stealing from another RBTreeArray
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *         RBTreeArray32<unsigned,double> tree32Copy=tree32; // tree32: {{,}}, tree32Copy: {{1,2},{3,4},{5,6}}
 * 
 * ~RBTreeArray();
 *     Destructor
 * 
 * bool Insert(const KeyType& key,const ValueType& value)noexcept;
 *     Insert a key and its corresponding value
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32;
 *         tree32.Insert(3,3.1415926);
 *     Return true if the key was successfully inserted or already existed, if the key already existed, its value will be replace
 *     Return false if the key count of the tree has hit the maximum of the array size and the key dose not existed in the tree
 * 
 * bool Delete(const KeyType& key)noexcept;
 *     Delete a key-value pair form the tree
 *     Usage example: 
 *         tree.Delete();
 *     Return true if key existed in the tree, false if key dose not existed in the tree
 * 
 * uint64_t ConditionalDelete(ConditionFunction&& condition,Parameters&&... parameters);
 *     Delete all key-value pairs that condition returns true, condition can be function pointer, std::function, lambda. parameters can be any type
 *     condition must receive at least key and value
 *     Return the number of key-value pairs deleted
 * 
 * uint64_t ConditionalDeleteOnce(ConditionFunction&& condition,Parameters&&... parameters)noexcept;
 *     Delete one key-value pair that condition returns true, condition can be function pointer, std::function, lambda. parameters can be any type
 *     condition must receive at least key and value
 *     Return the number of key-value pairs deleted
 *     If condition returns true on more than one key-value pairs, it is not guarantee that the delete is the minimum
 * 
 * bool Search(const KeyType& key,ValueType& value)const noexcept;
 *     Receive key and value, Search by key in tree, and store its corresponding value into value
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32;
 *         // ...
 *         unsigned key=3;
 *         double value;
 *         tree32.Search(key,value); // searched value store in value
 *     Return true if key existed in tree
 * 
 * bool GetMin(KeyType& key,ValueType& value)const noexcept;
 *     Get the minimum key and its corresponding value
 *     Return true if there is at least one key in tree
 * 
 * bool GetMax(KeyType& key,ValueType& value)const noexcept;
 *     Get the maximum key and its corresponding value
 *     Return true if there is at least one key in tree
 * 
 * bool GetSmallestGraterThan(const KeyType& key,KeyType& greater,ValueType& value)const noexcept;
 *     Get the smallest key and its corresponding value that greater than the giving key
 *     Return true if exist
 * 
 * bool GetBiggestSmallerThan(const KeyType& key,KeyType& smaller,ValueType& value)const noexcept;
 *     Get the biggest key and its corresponding value that smaller than the giving key
 *     Return true if exist
 * 
 * std::vector<KeyType> Keys()const;
 *     Get all keys
 * 
 * std::vector<ValueType> Values()const;
 *     Get all values
 * 
 * std::vector<std::pair<KeyType,ValueType>> KeysValues()const;
 *     Get all key-value pairs
 * 
 * std::vector<const KeyType*> KeysPointer()const;
 *     Get pointers of all keys
 *     Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.
 * 
 * std::vector<ValueType*> ValuesPointer()const;
 *     Get pointers of all values
 *     Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.
 * 
 * std::vector<std::pair<const KeyType*,ValueType*>> KeysValuesPointer()const;
 *     Get pointers of all key-value pairs
 *     Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.
 * 
 * bool MemoryShrink()noexcept;
 *     Shrink the array size to the key count of the tree
 *     return true if malloc success or key count == current array size
 * 
 * bool ReSize(uint64_t size);
 *     Resize the array size
 *     return true if malloc success or size == current array size
 * 
 * void Clear();
 *     Set tree to empty tree, will not release the memory
 *     Call Clear() first than MemoryShrink() to release the memory use
 * 
 * bool IsEmpty();
 *     Return true if tree is empty
 * 
 * RBTree * Data()const{return tree;}
 *     Return a C style pointer that point to the RBTree struct, call ByteSize() to get tha Byte size of the struct
 * 
 * bool SetTree(RBTree * another);
 *     Set this tree from another RBTree struct pointer, the bit length of this tree and another must be the same
 *     Warning: The key type and value type of this tree and another must be the same, or it will be undefined behavior
 *     Return true if the bit length is same
 *     Warning: After calling this function, my previous tree will be destoryed
 * 
 * bool SetTreeWithoutDestoryMyTree(RBTree * another);
 *     Set this tree from another RBTree struct pointer without destory my tree, the bit length of this tree and another must be the same
 *     Warning: The key type and value type of this tree and another must be the same, or it will be undefined behavior
 *     Return true if the bit length is same
 * 
 * uint64_t KeyCount()const;
 *     Return the key-value pair count
 * 
 * uint64_t ArraySize()const;
 *     Return the node array size of this RBTreeArray, greater or equal to the key count
 * 
 * uint64_t GetBitLength()const;
 *     Return the bit length
 * 
 * uint64_t SizeAvailable()const;
 *     Return the maximum number of key-value pair that can be inserted
 * 
 * bool Transform(const AnotherRBTreeArrayType& another);
 *     Transform the data from another tree with different bit length, after calling this function, this tree and another will have the same key-value data with different bit length
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}}; // tree32: {{1,2},{3,4},{5,6}}
 *         RBTreeArray16<unsigned,double> tree16;                     // tree16: {,}
 *         tree16.Transform(tree32)                                   // tree16: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
 *         // tree16 and tree32 now have the same key-value pairs but different bit length
 *     Return false if the KeyCount of another tree is greater than the maximum node number that this tree allowed or malloc failed
 * 
 * ValueType& operator[](const KeyType& key);
 *     Return the reference of the value paired to the key
 *     If the key does not exist, it will creat a node with the giving key
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *         tree32[7]=3.1415926;
 *         tree32[7]=2*3.1415926;
 *     throw std::out_of_range("RBTreeArray: Both search and insert failed when using operator []") if both search and insert have failed
 * 
 * RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);
 *     operator =
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *         RBTreeArray32<unsigned,double> tree32Copy;
 *         tree32Copy=tree32; // tree32Copy: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
 *     
 * RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);
 *     operator = with move copy
 *     Usage example: 
 *         RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
 *         RBTreeArray32<unsigned,double> tree32Steal;
 *         tree32Steal=std::move(tree32); // tree32Steal: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
 * 
 * 
 * Iterator:
 * ---------
 * 
 * UnorderedIterator:
 *     Iterator of the array order, this kind of traversal is much faster but unordered 
 * 
 * UnorderedIterator begin();
 *     Return UnorderedIterator at the begin of UnorderedIterator
 * 
 * UnorderedIterator end();
 *     Return UnorderedIterator at the end of UnorderedIterator
 * 
 * Usage example: 
 *     RBTreeArray32<std::string,std::vector<double>> tree;
 *     // ...
 *     for(auto iterator=tree.begin();iterator!=tree.end();++iterator){
 *         auto key=iterator.Key();
 *         suto value=iterator.Value();
 *     }
 *     for(auto iterator=tree.begin();iterator!=tree.end();iterator++){
 *         // ...
 *     }
 *     for(auto iterator=tree.begin();iterator!=tree.end();iterator=iterator+1){
 *         // ...
 *     }
 *     for(const auto& [key,value]:tree){ // need C++17
 *         // ...
 *     }
 * 
 * UnorderedIterator UnorderedBegin();
 *     Equals to begin()
 * 
 * UnorderedIterator UnorderedEnd();
 *     Equals to end()
 * 
 * const KeyType& UnorderedIterator::Key();
 * ValueType& UnorderedIterator::Value();
 *     Get key or value reference from UnorderedIterator
 *     Usage example: 
 *         RBTreeArray32<std::string,std::vector<double>> tree;
 *         // ...
 *         for(auto iterator=tree.begin();iterator!=tree.end();++iterator){
 *             auto key=iterator.Key();
 *             suto value=iterator.Value();
 *         }
 * 
 * OrderedIterator:
 *     Iterator of the key order, this kind of traversal is slower than UnorderedIterator but ordered
 * 
 * OrderedIterator OrderedBegin();
 *     Return OrderedIterator at the begin of OrderedIterator
 * 
 * OrderedIterator OrderedEnd();
 *     Return OrderedIterator at the end of OrderedIterator
 * 
 * Usage example: 
 *     RBTreeArray32<std::string,std::vector<double>> tree;
 *     // ...
 *     for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();++iterator){
 *         auto key=iterator.Key();
 *         suto value=iterator.Value();
 *     }
 *     for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();iterator++){
 *         // ...
 *     }
 * 
 * const KeyType& OrderedIterator::Key();
 * ValueType& OrderedIterator::Value();
 *     Get key or value reference from OrderedIterator
 *     Usage example: 
 *         RBTreeArray32<std::string,std::vector<double>> tree;
 *         // ...
 *         for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();++iterator){
 *             auto key=iterator.Key();
 *             suto value=iterator.Value();
 *         }
 */

#ifndef __RBTREE_ARRAY_CXX_H__
#define __RBTREE_ARRAY_CXX_H__

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <typeinfo>
#include <new> // Placement New
#include <stdexcept>
#include <vector>

#define likely(x)   __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

typedef struct RBTree{
	uint64_t nodeCount;
	uint64_t rootIndex;
	uint64_t size;
	uint64_t bitLength;
	char nodes[];
}RBTree;

template<typename Whatever>
struct RBTreeArrayTemplateBaseType;

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength=sizeof(IndexType)*8>
class RBTreeArray{
public:
	RBTreeArray();
	RBTreeArray(uint64_t size);
	RBTreeArray(std::initializer_list<std::pair<KeyType,ValueType>> initList);
	RBTreeArray(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);
	RBTreeArray(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);
	~RBTreeArray();
	bool Insert(const KeyType& key,const ValueType& value)noexcept;
	bool Delete(const KeyType& key)noexcept;
	template<typename ConditionFunction,typename... Parameters>
	uint64_t ConditionalDelete(ConditionFunction&& condition,Parameters&&... parameters);
	template<typename ConditionFunction,typename... Parameters>
	uint64_t ConditionalDeleteOnce(ConditionFunction&& condition,Parameters&&... parameters)noexcept;
	bool Search(const KeyType& key,ValueType& value)const noexcept;
	bool GetMin(KeyType& key,ValueType& value)const noexcept;
	bool GetMax(KeyType& key,ValueType& value)const noexcept;
	bool GetSmallestGraterThan(const KeyType& key,KeyType& greater,ValueType& value)const noexcept;
	bool GetBiggestSmallerThan(const KeyType& key,KeyType& smaller,ValueType& value)const noexcept;
	std::vector<KeyType> Keys()const;
	std::vector<ValueType> Values()const;
	std::vector<std::pair<KeyType,ValueType>> KeysValues()const;
	std::vector<const KeyType*> KeysPointer()const;
	std::vector<ValueType*> ValuesPointer()const;
	std::vector<std::pair<const KeyType*,ValueType*>> KeysValuesPointer()const;
	bool MemoryShrink()noexcept;
	bool ReSize(uint64_t size);
	void Clear();
	bool IsEmpty(){return !static_cast<bool>(KeyCount());}
	RBTree* Data()const{return tree;}
	uint64_t ByteSize()const{return sizeof(RBTree)+sizeof(Node)*ArraySize();}
	bool SetTree(RBTree* another);
	bool SetTreeWithoutDestoryMyTree(RBTree* another);
	uint64_t KeyCount()const{return tree->nodeCount;}
	uint64_t ArraySize()const{return tree->size;}
	uint64_t GetBitLength()const{return bitLength;}
	uint64_t SizeAvailable()const{return MaxNodeCount-KeyCount();}
	
	template<typename AnotherRBTreeArrayType>
	bool Transform(const AnotherRBTreeArrayType& another);

	ValueType& operator[](const KeyType& key);
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);

	class OrderedIterator{
	public:
		OrderedIterator():tree(tree),currentIndex(MaxNodeCount),reachedBegin(reachedBegin),reachedEnd(reachedEnd){}
		OrderedIterator(RBTree* tree,uint64_t currentIndex,bool reachedBegin=false,bool reachedEnd=false):tree(tree),currentIndex(currentIndex),reachedBegin(reachedBegin),reachedEnd(reachedEnd){}
		OrderedIterator& operator++();
		OrderedIterator& operator--();
		OrderedIterator operator++(int);
		OrderedIterator operator--(int);
		bool operator!=(const OrderedIterator& another)const;
		bool operator==(const OrderedIterator& another)const;

		const KeyType& Key();
		ValueType& Value();
	private:
		RBTree* tree;
		IndexType currentIndex;
		bool reachedEnd=false;
		bool reachedBegin=false;
	};

	class UnorderedIterator{
	public:
		UnorderedIterator():tree(tree),currentIndex(MaxNodeCount),reachedBegin(reachedBegin){}
		UnorderedIterator(RBTree* tree,uint64_t currentIndex,bool reachedBegin=false):tree(tree),currentIndex(currentIndex),reachedBegin(reachedBegin){}
		UnorderedIterator& operator++();
		UnorderedIterator& operator--();
		UnorderedIterator operator++(int);
		UnorderedIterator operator--(int);
		UnorderedIterator operator+(long long gap)const;
		UnorderedIterator operator-(long long gap)const;
		bool operator!=(const UnorderedIterator& another)const;
		bool operator==(const UnorderedIterator& another)const;
		std::pair<const KeyType&,ValueType&> operator*()const;

		const KeyType& Key();
		ValueType& Value();
	private:
		RBTree* tree;
		IndexType currentIndex;
		bool reachedBegin=false;
	};

	UnorderedIterator begin()const;
	UnorderedIterator end()const;
	UnorderedIterator UnorderedBegin()const;
	UnorderedIterator UnorderedEnd()const;
	OrderedIterator OrderedBegin()const;
	OrderedIterator OrderedEnd()const;

	static constexpr uint64_t MaxNodeCount=(BitLength==16)?0xFFFFLLU:(BitLength==32)?0xFFFFFFFFLLU:0xFFFFFFFFFFFFFFFFLLU;
	static constexpr unsigned bitLength=BitLength;
private:
	typedef struct RBTreeNode{
		IndexType fatherIndex;
		IndexType leftIndex;
		IndexType rightIndex;
		uint32_t color;
		KeyType key;
		ValueType value;

		struct RBTreeNode& operator=(struct RBTreeNode&& another)noexcept{
			fatherIndex=another.fatherIndex;
			leftIndex  =another.leftIndex;
			rightIndex =another.rightIndex;
			color      =another.color;
			key        =std::move(another.key);
			value      =std::move(another.value);
			return *(this);
		}
		struct RBTreeNode& operator=(const struct RBTreeNode& another)noexcept{
			fatherIndex=another.fatherIndex;
			leftIndex  =another.leftIndex;
			rightIndex =another.rightIndex;
			color      =another.color;
			key        =another.key;
			value      =another.value;
			return *(this);
		}
	}Node;

	typedef struct RBTreeNode16{
		uint16_t fatherIndex;
		uint16_t leftIndex;
		uint16_t rightIndex;
		uint32_t color;
		KeyType key;
		ValueType value;
	}Node16;

	typedef struct RBTreeNode32{
		uint32_t fatherIndex;
		uint32_t leftIndex;
		uint32_t rightIndex;
		uint32_t color;
		KeyType key;
		ValueType value;
	}Node32;

	typedef struct RBTreeNode64{
		uint64_t fatherIndex;
		uint64_t leftIndex;
		uint64_t rightIndex;
		uint32_t color;
		KeyType key;
		ValueType value;
	}Node64;

	uint64_t NodeCreate(uint64_t fatherIndex,const KeyType& key,const ValueType& value)noexcept;
	RBTree* CreateSize(uint64_t size)noexcept;
	bool InsertCore(Node* firstNode,Node* root,Node* current,Node* father,Node* grandfather)noexcept;
	unsigned GetRouteCase(const Node* firstNode,const Node* current,const Node* father,const Node* grandfather)noexcept;
	void DeleteNode(Node* nodes,Node* father,uint64_t toDeleteIndex,uint64_t** indexes,Node*** nodesToUpdate)noexcept;
	bool DeleteCore(const KeyType& key,IndexType* deleteIndex)noexcept;
	void FatherBrotherGrandFatherUpdate(uint64_t toMoveIndex,uint64_t toDeleteIndex,Node* nodes,uint64_t** indexes,Node*** nodesToUpdate)noexcept;
	void PlacementNew(Node* nodes,uint64_t size)noexcept;
	void PlacementDelete()noexcept;
	bool Assign(RBTree* destination,const RBTree* source,bool move=false);
	template<typename AnotherNodeType>
	void NodeAssign(Node* destination,const AnotherNodeType* source,uint64_t count,bool move);
	void TreeInformationAssign(RBTree* destination,const RBTree* source){
		destination->nodeCount=source->nodeCount;
		destination->rootIndex=source->rootIndex;
	}
	static IndexType GetMinIndex(RBTree* tree);
	static IndexType GetMaxIndex(RBTree* tree);
	void PrintInformation(); // this is for test
	void CheckColor(); // this is for test
	IndexType IndexSmallestGraterThan(const KeyType& key)const noexcept;
	IndexType IndexBiggestSmallerThan(const KeyType& key)const noexcept;

	template<typename AnotherRBTreeArrayType>
	void CheckTransformable(const AnotherRBTreeArrayType& another)const;
	template<typename AnotherRBTreeArrayType>
	void CheckAssignable(const AnotherRBTreeArrayType& another)const;

	static const uint64_t LeastNodeCount=256;
	static const uint64_t MaxNodeCount16=0xFFFFLLU;
	static const uint64_t MaxNodeCount32=0xFFFFFFFFLLU;
	static const uint64_t MaxNodeCount64=0xFFFFFFFFFFFFFFFFLLU;
	RBTree* tree=nullptr;

	enum class Color{
		Red=0,
		Black
	};

	enum class RouteCase{
		RR=0,
		RL,
		LR,
		LL
	};
};

template<typename KeyType,typename ValueType>
using RBTreeArray16=RBTreeArray<KeyType,ValueType,uint16_t,sizeof(uint16_t)*8>;

template<typename KeyType,typename ValueType>
using RBTreeArray32=RBTreeArray<KeyType,ValueType,uint32_t,sizeof(uint32_t)*8>;

template<typename KeyType,typename ValueType>
using RBTreeArray64=RBTreeArray<KeyType,ValueType,uint64_t,sizeof(uint64_t)*8>;

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
struct RBTreeArrayTemplateBaseType<RBTreeArray<KeyType,ValueType,IndexType,BitLength>>{
	using KeyTypeBase  =KeyType;
	using ValueTypeBase=ValueType;
	using IndexTypeBase=IndexType;
	static constexpr unsigned BitLengthBase=BitLength;
};

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::PrintInformation(){
	switch(bitLength){
	case 16:
		printf("RBTreeArray16:\n");
		break;
	case 32:
		printf("RBTreeArray32:\n");
		break;
	case 64:
		printf("RBTreeArray64:\n");
		break;
	default:
		printf("ERROR TYPE:\n");
		break;
	}
	printf("    KeyType  : %s\n",typeid(KeyType).name());
	printf("    ValueType: %s\n",typeid(ValueType).name());
	printf("    IndexType: %s\n",typeid(IndexType).name());
	printf("    nodeCount: %llu\n",(long long unsigned int)KeyCount());
	printf("    size     : %llu\n",(long long unsigned int)ArraySize());
	printf("    SizeAvail: %llu\n",(long long unsigned int)SizeAvailable());
	printf("    MaxNodeCount: %llu\n",(long long unsigned int)MaxNodeCount);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTree* RBTreeArray<KeyType,ValueType,IndexType,BitLength>::CreateSize(uint64_t size)noexcept{
	if(!size){
		size=1;
	}
	RBTree* tree=(RBTree*)malloc(sizeof(RBTree)+sizeof(Node)*(size&MaxNodeCount));
	if(tree){
		tree->nodeCount=0;
		tree->rootIndex=0;
		tree->size=size&MaxNodeCount;
		tree->bitLength=bitLength;
		PlacementNew(reinterpret_cast<Node*>(tree->nodes),tree->size);
		return tree;
	}
	return NULL;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::RBTreeArray():RBTreeArray(LeastNodeCount){
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::RBTreeArray(uint64_t size){
	if(size>MaxNodeCount){
		char buffer[1024];
		sprintf(buffer,"RBTreeArray: attempt to create RBTreeArray%u with size %llu has exceed its capacity",bitLength,size);
		throw std::out_of_range(buffer);
	}
	tree=CreateSize(size);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::RBTreeArray(std::initializer_list<std::pair<KeyType,ValueType>> initList){
	uint64_t size=initList.size();
	if(size<LeastNodeCount){
		size=LeastNodeCount;
	}
	if(size>MaxNodeCount){
		char buffer[1024];
		sprintf(buffer,"RBTreeArray: attempt to create RBTreeArray%u with size %llu has exceed its capacity",bitLength,size);
		throw std::out_of_range(buffer);
	}
	tree=CreateSize(size);
	for(const std::pair<KeyType,ValueType>& pair:initList){
		Insert(pair.first,pair.second);
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::RBTreeArray(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another):RBTreeArray(1){
	if(this!=&another){
		Transform(another);
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::RBTreeArray(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another):RBTreeArray(1){
	if(this!=&another){
		SetTree(another.Data());
		RBTree* newTree=CreateSize(0);
		another.SetTreeWithoutDestoryMyTree(newTree);
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::PlacementDelete()noexcept{
	if(std::is_fundamental<KeyType>::value&&std::is_fundamental<ValueType>::value){
		return;
	}
	Node* nodes=(Node*)(tree->nodes);
	for(uint64_t index=0;index<ArraySize();index=index+1){
		nodes[index].key.~KeyType();
		nodes[index].value.~ValueType();
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>::~RBTreeArray(){
	PlacementDelete();
	free(tree);
	tree=nullptr;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::PlacementNew(Node* nodes,uint64_t size)noexcept{
	if(std::is_fundamental<KeyType>::value&&std::is_fundamental<ValueType>::value){
		return;
	}
	for(uint64_t index=0;index<size;index=index+1){
		new(&(nodes[index].key))KeyType();
		new(&(nodes[index].value))ValueType();
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline uint64_t RBTreeArray<KeyType,ValueType,IndexType,BitLength>::NodeCreate(uint64_t fatherIndex,const KeyType& key,const ValueType& value)noexcept{
	uint64_t nodeCount=tree->nodeCount;
	if(unlikely(nodeCount==tree->size)){
		uint64_t size=tree->size;
		if(size==MaxNodeCount){
			return MaxNodeCount;
		}
		size=size<<1;
		if(size>MaxNodeCount){
			size=MaxNodeCount;
		}
		RBTree* newTree=CreateSize(size);
		Assign(newTree,tree,true);
		this->~RBTreeArray();
		tree=newTree;
	}
	Node* nodes=(Node*)(tree->nodes);
	nodes[nodeCount].fatherIndex=fatherIndex;
	nodes[nodeCount].key=key;
	nodes[nodeCount].value=value;
	nodes[nodeCount].leftIndex=MaxNodeCount;
	nodes[nodeCount].rightIndex=MaxNodeCount;
	nodes[nodeCount].color=static_cast<uint32_t>(Color::Red);
	tree->nodeCount=tree->nodeCount+1;
	return tree->nodeCount-1;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Insert(const KeyType& key,const ValueType& value)noexcept{
	Node* nodes=(Node*)(tree->nodes);
	if(unlikely(tree->nodeCount==0)){
		uint64_t rootIndex=NodeCreate(MaxNodeCount,key,value);
		tree->rootIndex=rootIndex;
		nodes=(Node*)(tree->nodes);
		nodes[rootIndex].color=static_cast<uint32_t>(Color::Black);
		return true;
	}
	Node* firstNode=(Node*)(tree->nodes);
	Node* current=nodes+tree->rootIndex;
	while(true){
		if(key>current->key){
			if(current->rightIndex==MaxNodeCount){
				if(unlikely(tree->nodeCount==MaxNodeCount)){
					return false;
				}
				uint64_t currentIndex=current-nodes;
				uint64_t rightIndex=NodeCreate(currentIndex,key,value);
				nodes=(Node*)(tree->nodes);
				current=nodes+currentIndex;
				current->rightIndex=rightIndex;
				current=nodes+rightIndex;
				break;
			}
			current=nodes+current->rightIndex;
			continue;
		}
		if(key<current->key){
			if(current->leftIndex==MaxNodeCount){
				if(unlikely(tree->nodeCount==MaxNodeCount)){
					return false;
				}
				uint64_t currentIndex=current-nodes;
				uint64_t leftIndex=NodeCreate(currentIndex,key,value);
				nodes=(Node*)(tree->nodes);
				current=nodes+currentIndex;
				current->leftIndex=leftIndex;
				current=nodes+leftIndex;
				break;
			}
			current=nodes+current->leftIndex;
			continue;
		}
		current->value=value;
		return true;
	}
	firstNode=(Node*)(tree->nodes);
	Node* root=firstNode+tree->rootIndex;
	Node* father=firstNode+current->fatherIndex;
	// RR==0 RL==1 LR==2 LL==3
	if(father->fatherIndex!=MaxNodeCount){
		Node* grandfather=firstNode+father->fatherIndex;
		Node* greatGrandfather=NULL;
		return InsertCore(firstNode,root,current,father,grandfather);
	}
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline unsigned RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetRouteCase(const Node* firstNode,const Node* current,const Node* father,const Node* grandfather)noexcept{
	if(grandfather->leftIndex==father-firstNode){
		if(father->leftIndex==current-firstNode){
			return static_cast<unsigned>(RouteCase::LL);
		}
		return static_cast<unsigned>(RouteCase::LR);
	}
	if(father->leftIndex==current-firstNode){
		return static_cast<unsigned>(RouteCase::RL);
	}
	return static_cast<unsigned>(RouteCase::RR);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::InsertCore(Node* firstNode,Node* root,Node* current,Node* father,Node* grandfather)noexcept{
	unsigned routeCase;
	Node* greatGrandfather;
	while((current->color==static_cast<uint32_t>(Color::Red))&&(father->color==static_cast<uint32_t>(Color::Red))){
		routeCase=GetRouteCase(firstNode,current,father,grandfather);
		switch(routeCase){
		case static_cast<unsigned>(RouteCase::RR):
			if(grandfather->leftIndex!=MaxNodeCount){
				if((firstNode+grandfather->leftIndex)->color==static_cast<uint32_t>(Color::Red)){
					goto redUncle;
				}
			}
			grandfather->rightIndex=father->leftIndex;
			if(grandfather->rightIndex!=MaxNodeCount){
				(firstNode+grandfather->rightIndex)->fatherIndex=grandfather-firstNode;
			}
			father->leftIndex=grandfather-firstNode;
			greatGrandfather=firstNode+grandfather->fatherIndex;
			grandfather->fatherIndex=father-firstNode;
			father->fatherIndex=greatGrandfather-firstNode;
			if(grandfather==root){
				tree->rootIndex=father-firstNode;
			}else{
				if(firstNode+greatGrandfather->leftIndex==grandfather){
					greatGrandfather->leftIndex=father-firstNode;
				}else{
					greatGrandfather->rightIndex=father-firstNode;
				}
			}
			father->color=static_cast<uint32_t>(Color::Black);
			grandfather->color=static_cast<uint32_t>(Color::Red);
			return true;
		case static_cast<unsigned>(RouteCase::RL):
			if(grandfather->leftIndex!=MaxNodeCount){
				if((firstNode+grandfather->leftIndex)->color==static_cast<uint32_t>(Color::Red)){
					goto redUncle;
				}
			}
			father->leftIndex=current->rightIndex;
			if(father->leftIndex!=MaxNodeCount){
				(firstNode+father->leftIndex)->fatherIndex=father-firstNode;
			}
			grandfather->rightIndex=current->leftIndex;
			if(grandfather->rightIndex!=MaxNodeCount){
				(firstNode+grandfather->rightIndex)->fatherIndex=grandfather-firstNode;
			}
			current->leftIndex=grandfather-firstNode;
			current->rightIndex=father-firstNode;
			greatGrandfather=firstNode+grandfather->fatherIndex;
			current->fatherIndex=greatGrandfather-firstNode;
			father->fatherIndex=current-firstNode;
			grandfather->fatherIndex=current-firstNode;
			if(grandfather==root){
				tree->rootIndex=current-firstNode;
			}else{
				if(greatGrandfather->leftIndex==grandfather-firstNode){
					greatGrandfather->leftIndex=current-firstNode;
				}else{
					greatGrandfather->rightIndex=current-firstNode;
				}
			}
			current->color=static_cast<uint32_t>(Color::Black);
			grandfather->color=static_cast<uint32_t>(Color::Red);
			return true;
		case static_cast<unsigned>(RouteCase::LR):
			if(grandfather->rightIndex!=MaxNodeCount){
				if((firstNode+grandfather->rightIndex)->color==static_cast<uint32_t>(Color::Red)){
					goto redUncle;
				}
			}
			father->rightIndex=current->leftIndex;
			if(father->rightIndex!=MaxNodeCount){
				(firstNode+father->rightIndex)->fatherIndex=father-firstNode;
			}
			grandfather->leftIndex=current->rightIndex;
			if(grandfather->leftIndex!=MaxNodeCount){
				(firstNode+grandfather->leftIndex)->fatherIndex=grandfather-firstNode;
			}
			current->leftIndex=father-firstNode;
			current->rightIndex=grandfather-firstNode; 
			greatGrandfather=firstNode+grandfather->fatherIndex;
			current->fatherIndex=greatGrandfather-firstNode;
			father->fatherIndex=current-firstNode;
			grandfather->fatherIndex=current-firstNode;
			if(grandfather==root){
				tree->rootIndex=current-firstNode;
			}else{
				if(greatGrandfather->leftIndex==grandfather-firstNode){
					greatGrandfather->leftIndex=current-firstNode;
				}else{
					greatGrandfather->rightIndex=current-firstNode;
				}
			}
			current->color=static_cast<uint32_t>(Color::Black);
			grandfather->color=static_cast<uint32_t>(Color::Red);
			return true;
		case static_cast<unsigned>(RouteCase::LL):
			if(grandfather->rightIndex!=MaxNodeCount){
				if((firstNode+grandfather->rightIndex)->color==static_cast<uint32_t>(Color::Red)){
					goto redUncle;
				}
			}
			grandfather->leftIndex=father->rightIndex;
			if(grandfather->leftIndex!=MaxNodeCount){
				(firstNode+grandfather->leftIndex)->fatherIndex=grandfather-firstNode;
			}
			father->rightIndex=grandfather-firstNode;
			greatGrandfather=firstNode+grandfather->fatherIndex;
			father->fatherIndex=greatGrandfather-firstNode;
			grandfather->fatherIndex=father-firstNode;
			if(root==grandfather){
				tree->rootIndex=father-firstNode;
			}else{
				if(greatGrandfather->leftIndex==grandfather-firstNode){
					greatGrandfather->leftIndex=father-firstNode;
				}else{
					greatGrandfather->rightIndex=father-firstNode;
				}
			}
			father->color=static_cast<uint32_t>(Color::Black);
			grandfather->color=static_cast<uint32_t>(Color::Red);
			return true;
		default:
			return false;
		}
	redUncle:
		grandfather->color=static_cast<uint32_t>(Color::Red);
		(firstNode+grandfather->leftIndex)->color=static_cast<uint32_t>(Color::Black);
		(firstNode+grandfather->rightIndex)->color=static_cast<uint32_t>(Color::Black);
		current=firstNode+((firstNode+current->fatherIndex)->fatherIndex);
		if(current-firstNode==tree->rootIndex||current->fatherIndex==tree->rootIndex){
			(firstNode+tree->rootIndex)->color=static_cast<uint32_t>(Color::Black);
			return true;
		}
		grandfather=firstNode+((firstNode+current->fatherIndex)->fatherIndex);
		father=firstNode+current->fatherIndex;
	}
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::FatherBrotherGrandFatherUpdate(uint64_t toMoveIndex,uint64_t toDeleteIndex,Node* nodes,uint64_t** indexes,Node*** nodesToUpdate)noexcept{
	// Loop unwinding
	uint64_t changeIndex=MaxNodeCount;
	if(*(indexes[0])==toMoveIndex){
		changeIndex=*(indexes[0]);
		*(indexes[0])=toDeleteIndex;
		*(nodesToUpdate[0])=nodes+toDeleteIndex;
		goto checkRoot;
	}
	if(*(indexes[1])==toMoveIndex){
		changeIndex=*(indexes[1]);
		*(indexes[1])=toDeleteIndex;
		*(nodesToUpdate[1])=nodes+toDeleteIndex;
		goto checkRoot;
	}
	if(*(indexes[2])==toMoveIndex){
		changeIndex=*(indexes[2]);
		*(indexes[2])=toDeleteIndex;
		*(nodesToUpdate[2])=nodes+toDeleteIndex;
		goto checkRoot;
	}
	return;
	checkRoot:
	if(tree->rootIndex==changeIndex){
		tree->rootIndex=toDeleteIndex;
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::DeleteNode(Node* nodes,Node* father,uint64_t toDeleteIndex,uint64_t** indexes,Node*** nodesToUpdate)noexcept{
	if(father->leftIndex==toDeleteIndex){
		father->leftIndex=MaxNodeCount;
	}else{
		father->rightIndex=MaxNodeCount;
	}
	long long unsigned int toMove=tree->nodeCount-1;
	if(likely(toMove!=toDeleteIndex)){
		if(toMove!=tree->rootIndex){
			if(nodes[nodes[toMove].fatherIndex].leftIndex==toMove){
				nodes[nodes[toMove].fatherIndex].leftIndex=toDeleteIndex;
			}else{
				nodes[nodes[toMove].fatherIndex].rightIndex=toDeleteIndex;
			}
		}else{
			tree->rootIndex=toDeleteIndex;
		}
		FatherBrotherGrandFatherUpdate(toMove,toDeleteIndex,nodes,indexes,nodesToUpdate);
		if(nodes[toMove].leftIndex!=MaxNodeCount){
			nodes[nodes[toMove].leftIndex].fatherIndex=toDeleteIndex;
		}
		if(nodes[toMove].rightIndex!=MaxNodeCount){
			nodes[nodes[toMove].rightIndex].fatherIndex=toDeleteIndex;
		}
		nodes[toDeleteIndex]=std::move(nodes[toMove]);
	}
	tree->nodeCount=tree->nodeCount-1;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::DeleteCore(const KeyType& key,IndexType* deleteIndex)noexcept{
	Node* nodes=(Node*)(tree->nodes);
	Node* current=nodes+tree->rootIndex;
	if(unlikely(tree->nodeCount==1)){
		if(key==current->key){
			tree->rootIndex=0;
			tree->nodeCount=0;
			*(deleteIndex)=0;
			return true;
		}
		return false;
	}
	while(true){
		if(key>current->key){
			if(current->rightIndex==MaxNodeCount){
				return false;
			}
			current=nodes+current->rightIndex;
			continue;
		}
		if(key<current->key){
			if(current->leftIndex==MaxNodeCount){
				return false;
			}
			current=nodes+current->leftIndex;
			continue;
		}
		break;
	}
	bool deleted=false;
	Node* brother;
	Node* father=nodes+current->fatherIndex;
	Node* grandfather;
	uint64_t currentIndex=current-nodes;
	uint64_t myFatherIndex=current->fatherIndex;
	uint64_t myBrotherIndex=MaxNodeCount;
	uint64_t myGrandfatherIndex=MaxNodeCount;
	uint64_t* indexes[]={&myFatherIndex,&myBrotherIndex,&myGrandfatherIndex};
	Node** nodesToUpdate[]={&father,&brother,&grandfather};
	if(unlikely(current->fatherIndex==MaxNodeCount)){
		if(current->leftIndex==MaxNodeCount){
			current->key=(nodes+current->rightIndex)->key;
			current->value=(nodes+current->rightIndex)->value;
			*(deleteIndex)=current->rightIndex;
			DeleteNode(nodes,current,current->rightIndex,indexes,nodesToUpdate);
			return true;
		}else{
			if(current->rightIndex==MaxNodeCount){
				current->key=(nodes+current->leftIndex)->key;
				current->value=(nodes+current->leftIndex)->value;
				*(deleteIndex)=current->leftIndex;
				DeleteNode(nodes,current,current->leftIndex,indexes,nodesToUpdate);
				return true;
			}else{
				goto gotoRightSmallest;
			}
		}
	}
	if(current->leftIndex==MaxNodeCount){
		deleteBegin:
		if(current->rightIndex==MaxNodeCount){
			// No child
			if(current->color==static_cast<uint32_t>(Color::Red)){
				*(deleteIndex)=currentIndex;
				DeleteNode(nodes,father,currentIndex,indexes,nodesToUpdate);
				return true;
			}
			doubleBlackFix:
			grandfather=nodes+father->fatherIndex;
			myGrandfatherIndex=father->fatherIndex;
			if(father->leftIndex==currentIndex){
				brother=nodes+father->rightIndex;
				myBrotherIndex=father->rightIndex;
				if(brother->color==static_cast<uint32_t>(Color::Black)){
					if(brother->rightIndex!=MaxNodeCount){
						if((nodes+brother->rightIndex)->color==static_cast<uint32_t>(Color::Red)){
							// case RR
							if(!deleted){
								*(deleteIndex)=father->leftIndex;
								DeleteNode(nodes,father,father->leftIndex,indexes,nodesToUpdate);
							}
							(nodes+brother->rightIndex)->color=static_cast<uint32_t>(Color::Black);
							brother->color=father->color;
							father->color=static_cast<uint32_t>(Color::Black);
							father->rightIndex=brother->leftIndex;
							if(father->rightIndex!=MaxNodeCount){
								(nodes+father->rightIndex)->fatherIndex=myFatherIndex;
							}
							brother->leftIndex=myFatherIndex;
							father->fatherIndex=myBrotherIndex;
							brother->fatherIndex=myGrandfatherIndex;
							if(tree->rootIndex==myFatherIndex){
								tree->rootIndex=myBrotherIndex;
							}else{
								if(grandfather->leftIndex==myFatherIndex){
									grandfather->leftIndex=myBrotherIndex;
								}else{
									grandfather->rightIndex=myBrotherIndex;
								}
							}
							return true;
						}
					}
					if(brother->leftIndex!=MaxNodeCount){
						if((nodes+brother->leftIndex)->color==static_cast<uint32_t>(Color::Red)){
							// case RL
							if(!deleted){
								*(deleteIndex)=father->leftIndex;
								DeleteNode(nodes,father,father->leftIndex,indexes,nodesToUpdate);
							}
							Node* leftChild=nodes+brother->leftIndex;
							leftChild->color=father->color;
							father->color=static_cast<uint32_t>(Color::Black);
							father->rightIndex=leftChild->leftIndex;
							if(father->rightIndex!=MaxNodeCount){
								(nodes+father->rightIndex)->fatherIndex=myFatherIndex;
							}
							leftChild->leftIndex=myFatherIndex;
							father->fatherIndex=leftChild-nodes;
							leftChild->fatherIndex=myGrandfatherIndex;
							if(tree->rootIndex==myFatherIndex){
								tree->rootIndex=leftChild-nodes;
							}else{
								if(grandfather->leftIndex==myFatherIndex){
									grandfather->leftIndex=leftChild-nodes;
								}else{
									grandfather->rightIndex=leftChild-nodes;
								}
							}
							brother->leftIndex=leftChild->rightIndex;
							if(brother->leftIndex!=MaxNodeCount){
								(nodes+brother->leftIndex)->fatherIndex=myBrotherIndex;
							}
							leftChild->rightIndex=myBrotherIndex;
							brother->fatherIndex=leftChild-nodes;
							return true;
						}
					}
					if(!deleted){
						*(deleteIndex)=father->leftIndex;
						DeleteNode(nodes,father,father->leftIndex,indexes,nodesToUpdate);
					}
					goto brotherChildBothBlack;
				}else{
					brother->color=static_cast<uint32_t>(Color::Black);
					father->color=static_cast<uint32_t>(Color::Red);
					father->rightIndex=brother->leftIndex;
					if(father->rightIndex!=MaxNodeCount){
						(nodes+father->rightIndex)->fatherIndex=myFatherIndex;
					}
					father->fatherIndex=myBrotherIndex;
					brother->leftIndex=myFatherIndex;
					brother->fatherIndex=myGrandfatherIndex;
					if(tree->rootIndex==myFatherIndex){
						tree->rootIndex=myBrotherIndex;
					}else{
						if(grandfather->leftIndex==myFatherIndex){
							grandfather->leftIndex=myBrotherIndex;
						}else{
							grandfather->rightIndex=myBrotherIndex;
						}
					}
					goto doubleBlackFix;
				}
			}else{
				brother=nodes+father->leftIndex;
				myBrotherIndex=father->leftIndex;
				if(brother->color==static_cast<uint32_t>(Color::Black)){
					if(brother->leftIndex!=MaxNodeCount){
						if((nodes+brother->leftIndex)->color==static_cast<uint32_t>(Color::Red)){
							// case LL
							if(!deleted){
								*(deleteIndex)=father->rightIndex;
								DeleteNode(nodes,father,father->rightIndex,indexes,nodesToUpdate);
							}
							(nodes+brother->leftIndex)->color=static_cast<uint32_t>(Color::Black);
							brother->color=father->color;
							father->color=static_cast<uint32_t>(Color::Black);
							father->leftIndex=brother->rightIndex;
							if(father->leftIndex!=MaxNodeCount){
								(nodes+father->leftIndex)->fatherIndex=myFatherIndex;
							}
							brother->rightIndex=myFatherIndex;
							father->fatherIndex=myBrotherIndex;
							brother->fatherIndex=myGrandfatherIndex;
							if(tree->rootIndex==myFatherIndex){
								tree->rootIndex=myBrotherIndex;
							}else{
								if(grandfather->leftIndex==myFatherIndex){
									grandfather->leftIndex=myBrotherIndex;
								}else{
									grandfather->rightIndex=myBrotherIndex;
								}
							}
							return true;
						}
					}
					if(brother->rightIndex!=MaxNodeCount){
						if((nodes+brother->rightIndex)->color==static_cast<uint32_t>(Color::Red)){
							// case LR
							if(!deleted){
								*(deleteIndex)=father->rightIndex;
								DeleteNode(nodes,father,father->rightIndex,indexes,nodesToUpdate);
							}
							Node* rightChild=nodes+brother->rightIndex;
							rightChild->color=father->color;
							father->color=static_cast<uint32_t>(Color::Black);
							brother->rightIndex=rightChild->leftIndex;
							if(brother->rightIndex!=MaxNodeCount){
								(nodes+brother->rightIndex)->fatherIndex=myBrotherIndex;
							}
							rightChild->leftIndex=myBrotherIndex;
							brother->fatherIndex=rightChild-nodes;
							rightChild->fatherIndex=myGrandfatherIndex;
							if(tree->rootIndex==myFatherIndex){
								tree->rootIndex=rightChild-nodes;
							}else{
								if(grandfather->leftIndex==myFatherIndex){
									grandfather->leftIndex=rightChild-nodes;
								}else{
									grandfather->rightIndex=rightChild-nodes;
								}
							}
							father->leftIndex=rightChild->rightIndex;
							if(father->leftIndex!=MaxNodeCount){
								(nodes+father->leftIndex)->fatherIndex=myFatherIndex;
							}
							rightChild->rightIndex=myFatherIndex;
							father->fatherIndex=rightChild-nodes;
							return true;
						}
					}
					// both of the brother's childern are null or black, and brother is black
					if(!deleted){
						*(deleteIndex)=father->rightIndex;
						DeleteNode(nodes,father,father->rightIndex,indexes,nodesToUpdate);
					}
					brotherChildBothBlack:
					brother->color=static_cast<uint32_t>(Color::Red);
					if(unlikely(tree->rootIndex==myFatherIndex)){
						return true;
					}
					if(father->color==static_cast<uint32_t>(Color::Red)){
						father->color=static_cast<uint32_t>(Color::Black);
						return true;
					}
					current=father;
					currentIndex=current-nodes;
					deleted=true;
					father=nodes+current->fatherIndex;
					myFatherIndex=current->fatherIndex;
					grandfather=nodes+father->fatherIndex;
					myGrandfatherIndex=father->fatherIndex;
					goto doubleBlackFix;
				}else{
					brother->color=static_cast<uint32_t>(Color::Black);
					father->color=static_cast<uint32_t>(Color::Red);
					father->leftIndex=brother->rightIndex;
					if(father->leftIndex!=MaxNodeCount){
						(nodes+father->leftIndex)->fatherIndex=myFatherIndex;
					}
					father->fatherIndex=myBrotherIndex;
					brother->rightIndex=myFatherIndex;
					brother->fatherIndex=myGrandfatherIndex;
					if(tree->rootIndex==myFatherIndex){
						tree->rootIndex=myBrotherIndex;
					}else{
						if(grandfather->leftIndex==myFatherIndex){
							grandfather->leftIndex=myBrotherIndex;
						}else{
							grandfather->rightIndex=myBrotherIndex;
						}
					}
					goto doubleBlackFix;
				}
			}
		}
		// no left child but right child
		current->key=(nodes+current->rightIndex)->key;
		current->value=(nodes+current->rightIndex)->value;
		*(deleteIndex)=current->rightIndex;
		DeleteNode(nodes,current,current->rightIndex,indexes,nodesToUpdate);
		return true;
	}else{
		if(current->rightIndex==MaxNodeCount){
			// no right child but left child
			current->key=(nodes+current->leftIndex)->key;
			current->value=(nodes+current->leftIndex)->value;
			*(deleteIndex)=current->leftIndex;
			DeleteNode(nodes,current,current->leftIndex,indexes,nodesToUpdate);
			return true;
		}
		// left child and right child
		gotoRightSmallest:
		Node* rightSmallest=nodes+current->rightIndex;
		while(rightSmallest->leftIndex!=MaxNodeCount){
			rightSmallest=nodes+rightSmallest->leftIndex;
		}
		current->key=rightSmallest->key;
		current->value=rightSmallest->value;
		current=rightSmallest;
		currentIndex=rightSmallest-nodes;
		father=nodes+current->fatherIndex;
		myFatherIndex=current->fatherIndex;
		grandfather=nodes+father->fatherIndex;
		myGrandfatherIndex=father->fatherIndex;
		goto deleteBegin;
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Delete(const KeyType& key)noexcept{
	if(!tree){
		return false;
	}
	if(tree->nodeCount==0){
		return false;
	}
	IndexType deleteIndex;
	return DeleteCore(key,&deleteIndex);;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename ConditionFunction,typename... Parameters>
inline uint64_t RBTreeArray<KeyType,ValueType,IndexType,BitLength>::ConditionalDelete(ConditionFunction&& condition,Parameters&&... parameters){
	uint64_t deleted=0;
	uint64_t needToDelete=0;
	uint64_t notToDeleteIndex=0;
	double deleteRate;
	const double UnlikelyToDeleRate=0.25;
	const double NormalDeleRate=0.5;
	Node* nodes=(Node*)(tree->nodes);
	IndexType* notToDeleteIndeices=(IndexType*)malloc(sizeof(IndexType)*KeyCount());
	if(!notToDeleteIndeices){
		goto normalDelete;
	}
	for(IndexType index=0;index<KeyCount();index=index+1){
		if(condition(nodes[index].key,nodes[index].value,std::forward<Parameters>(parameters)...)){
			needToDelete=needToDelete+1;
		}else{
			notToDeleteIndeices[notToDeleteIndex]=index;
			notToDeleteIndex=notToDeleteIndex+1;
		}
	}
	deleteRate=double(needToDelete)/double(KeyCount());
	if(deleteRate<UnlikelyToDeleRate){
		for(IndexType index=0;index<KeyCount();index=index+1){
			if(condition(nodes[index].key,nodes[index].value,std::forward<Parameters>(parameters)...)){
				IndexType deleteIndex;
				if(DeleteCore(nodes[index].key,&deleteIndex)){
					deleted=deleted+1;
				}
			}
		}
		std::vector<KeyType> toDelete;
		toDelete.reserve(needToDelete-deleted);
		for(IndexType index=0;index<KeyCount();index=index+1){
			if(condition(nodes[index].key,nodes[index].value,std::forward<Parameters>(parameters)...)){
				toDelete.push_back(nodes[index].key);
			}
		}
		for(const auto& key:toDelete){
			IndexType deleteIndex;
			if(DeleteCore(key,&deleteIndex)){
				deleted=deleted+1;
			}
		}
	}else if(deleteRate<NormalDeleRate){
		normalDelete:
		IndexType index=GetMinIndex(tree);
		Node* nodes=(Node*)(tree->nodes);
		KeyType deletedKey;
		while(index!=MaxNodeCount){
			if(condition(nodes[index].key,nodes[index].value,std::forward<Parameters>(parameters)...)){
				deletedKey=nodes[index].key;
				IndexType deleteIndex;
				if(DeleteCore(nodes[index].key,&deleteIndex)){
					deleted=deleted+1;
				}
				index=IndexSmallestGraterThan(deletedKey);
			}else{
				Node* current=nodes+index;
				if(current->rightIndex!=MaxNodeCount){
					current=nodes+current->rightIndex;
					while(current->leftIndex!=MaxNodeCount){
						current=nodes+current->leftIndex;
					}
					index=current-nodes;
				}else{
					while(true){
						if(current->fatherIndex==MaxNodeCount){
							index=MaxNodeCount;
							break;
						}
						if((nodes+current->fatherIndex)->rightIndex!=current-nodes){
							index=current->fatherIndex;
							break;
						}
						current=nodes+current->fatherIndex;
					}
				}
			}
		}
	}else{
		RBTreeArray<KeyType,ValueType,IndexType,BitLength> newTree(ArraySize());
		if(!newTree.Data()){
			goto normalDelete;
		}
		for(IndexType index=0;index<KeyCount()-needToDelete;index=index+1){
			ValueType searchValue;
			newTree.Insert(nodes[notToDeleteIndeices[index]].key,nodes[notToDeleteIndeices[index]].value);
		}
		deleted=KeyCount()-newTree.KeyCount();
		*(this)=std::move(newTree);
	}
	free(notToDeleteIndeices);
	return deleted;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename ConditionFunction,typename... Parameters>
inline uint64_t RBTreeArray<KeyType,ValueType,IndexType,BitLength>::ConditionalDeleteOnce(ConditionFunction&& condition,Parameters&&... parameters)noexcept{
	uint64_t deleted=0;
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		if(condition(nodes[index].key,nodes[index].value,std::forward<Parameters>(parameters)...)){
			if(Delete(nodes[index].key)){
				deleted=deleted+1;
			}
			break;
		}
	}
	return deleted;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Search(const KeyType& key,ValueType& value)const noexcept{
	if(!KeyCount()){
		return false;
	}
	Node* nodes=(Node*)(tree->nodes);
	Node* current=nodes+tree->rootIndex;
	while(true){
		if(key>current->key){
			if(current->rightIndex==MaxNodeCount){
				return false;
			}
			current=nodes+current->rightIndex;
			continue;
		}
		if(key<current->key){
			if(current->leftIndex==MaxNodeCount){
				return false;
			}
			current=nodes+current->leftIndex;
			continue;
		}
		value=current->value;
		return true;
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMin(KeyType& key,ValueType& value)const noexcept{
	if(!tree->nodeCount){
		return false;
	}
	Node* current=(Node*)(tree->nodes)+tree->rootIndex;
	Node* nodes=(Node*)(tree->nodes);
	while(current->leftIndex!=MaxNodeCount){
		current=nodes+current->leftIndex;
	}
	key=current->key;
	value=current->value;
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMax(KeyType& key,ValueType& value)const noexcept{
	if(!tree->nodeCount){
		return false;
	}
	Node* current=(Node*)(tree->nodes)+tree->rootIndex;
	Node* nodes=(Node*)(tree->nodes);
	while(current->rightIndex!=MaxNodeCount){
		current=nodes+current->rightIndex;
	}
	key=current->key;
	value=current->value;
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<KeyType> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Keys()const{
	std::vector<KeyType> Keys;
	Keys.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		Keys.push_back(nodes[index].key);
	}
	return Keys;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<ValueType> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Values()const{
	std::vector<ValueType> Values;
	Values.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		Values.push_back(nodes[index].value);
	}
	return Values;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<std::pair<KeyType,ValueType>> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::KeysValues()const{
	std::vector<std::pair<KeyType,ValueType>> KeysValues;
	KeysValues.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		KeysValues.emplace_back(nodes[index].key,nodes[index].value);
	}
	return KeysValues;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<const KeyType*> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::KeysPointer()const{
	std::vector<KeyType> Keys;
	Keys.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		Keys.push_back(&(nodes[index].key));
	}
	return Keys;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<ValueType*> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::ValuesPointer()const{
	std::vector<ValueType> Values;
	Values.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		Values.push_back(&(nodes[index].value));
	}
	return Values;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::vector<std::pair<const KeyType*,ValueType*>> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::KeysValuesPointer()const{
	std::vector<std::pair<KeyType,ValueType>> KeysValues;
	KeysValues.reserve(KeyCount());
	Node* nodes=(Node*)(tree->nodes);
	for(IndexType index=0;index<KeyCount();index=index+1){
		KeysValues.emplace_back(&(nodes[index].key),&(nodes[index].value));
	}
	return KeysValues;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::ReSize(uint64_t size){
	if(size<KeyCount()){
		return false;
	}
	if(size==ArraySize()){
		return true;
	}
	if(size>MaxNodeCount){
		char buffer[1024];
		sprintf(buffer,"RBTreeArray: attempt to create RBTreeArray%u with size %llu has exceed its capacity",bitLength,size);
		throw std::out_of_range(buffer);
	}
	RBTree* newTree=CreateSize(size);
	if(newTree){
		Assign(newTree,tree,true);
		this->~RBTreeArray();
		tree=newTree;
		return true;
	}else{
		return false;
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::MemoryShrink()noexcept{
	return ReSize(KeyCount());
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Clear(){
	PlacementDelete();
	Node* nodes=(Node*)(tree->nodes);
	PlacementNew(nodes,tree->size);
	tree->nodeCount=0;
	tree->rootIndex=0;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline ValueType& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::operator[](const KeyType& key){
	Node* nodes=(Node*)(tree->nodes);
	ValueType value;
	if(unlikely(tree->nodeCount==0)){
		uint64_t rootIndex=NodeCreate(MaxNodeCount,key,value);
		tree->rootIndex=rootIndex;
		nodes=(Node*)(tree->nodes);
		nodes[rootIndex].color=static_cast<uint32_t>(Color::Black);
		return nodes[rootIndex].value;
	}
	Node* firstNode=(Node*)(tree->nodes);
	Node* current=nodes+tree->rootIndex;
	while(true){
		if(key>current->key){
			if(current->rightIndex==MaxNodeCount){
				if(unlikely(tree->nodeCount==MaxNodeCount)){
					throw std::out_of_range("RBTreeArray: Both search and insert failed when using operator []");
				}
				uint64_t currentIndex=current-nodes;
				uint64_t rightIndex=NodeCreate(currentIndex,key,value);
				nodes=(Node*)(tree->nodes);
				current=nodes+currentIndex;
				current->rightIndex=rightIndex;
				current=nodes+rightIndex;
				break;
			}
			current=nodes+current->rightIndex;
			continue;
		}
		if(key<current->key){
			if(current->leftIndex==MaxNodeCount){
				if(unlikely(tree->nodeCount==MaxNodeCount)){
					throw std::out_of_range("RBTreeArray: Both search and insert failed when using operator []");
				}
				uint64_t currentIndex=current-nodes;
				uint64_t leftIndex=NodeCreate(current-nodes,key,value);
				nodes=(Node*)(tree->nodes);
				current=nodes+currentIndex;
				current->leftIndex=leftIndex;
				current=nodes+leftIndex;
				break;
			}
			current=nodes+current->leftIndex;
			continue;
		}
		return current->value;
	}
	firstNode=(Node*)(tree->nodes);
	Node* root=firstNode+tree->rootIndex;
	Node* father=firstNode+current->fatherIndex;
	// RR==0 RL==1 LR==2 LL==3
	if(father->fatherIndex!=MaxNodeCount){
		Node* grandfather=firstNode+father->fatherIndex;
		Node* greatGrandfather=NULL;
		InsertCore(firstNode,root,current,father,grandfather);
	}
	return current->value;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename AnotherRBTreeArrayType>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::CheckTransformable(const AnotherRBTreeArrayType& another)const{
	using AnotherType=RBTreeArrayTemplateBaseType<AnotherRBTreeArrayType>;
	static_assert(std::is_same<KeyType,typename AnotherType::KeyTypeBase>::value,"RBTreeArray: Key must be same type when using Transform()");
	static_assert(std::is_same<ValueType,typename AnotherType::ValueTypeBase>::value,"RBTreeArray: Value must be same type when using Transform()");
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename AnotherRBTreeArrayType>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::CheckAssignable(const AnotherRBTreeArrayType& another)const{
	using AnotherType=RBTreeArrayTemplateBaseType<AnotherRBTreeArrayType>;
	static_assert(std::is_same<IndexType,typename AnotherType::IndexTypeBase>::value,"RBTreeArray: Bit length must be the same when using assign");
	static_assert(std::is_same<KeyType,typename AnotherType::KeyTypeBase>::value,"RBTreeArray: Key must be same type when using assign");
	static_assert(std::is_same<ValueType,typename AnotherType::ValueTypeBase>::value,"RBTreeArray: Value must be same type when using assign");
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename AnotherNodeType>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::NodeAssign(Node* destination,const AnotherNodeType* source,uint64_t count,bool move){
	if(move){
		for(uint64_t index=0;index<count;index=index+1){
			destination[index].fatherIndex=source[index].fatherIndex;
			destination[index].leftIndex  =source[index].leftIndex;
			destination[index].rightIndex =source[index].rightIndex;
			destination[index].color      =source[index].color;
			destination[index].key        =std::move(source[index].key);
			destination[index].value      =std::move(source[index].value);
		}
	}else{
		for(uint64_t index=0;index<count;index=index+1){
			destination[index].fatherIndex=source[index].fatherIndex;
			destination[index].leftIndex  =source[index].leftIndex;
			destination[index].rightIndex =source[index].rightIndex;
			destination[index].color      =source[index].color;
			destination[index].key        =source[index].key;
			destination[index].value      =source[index].value;
		}
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Assign(RBTree* destination,const RBTree* source,bool move){
	if(source->nodeCount>destination->size){
		return false;
	}
	Node* nodesDestination=(Node*)(destination->nodes);
	switch(source->bitLength){
	case sizeof(uint16_t)*8:{
		const Node16* nodesSource=(const Node16*)(source->nodes);
		NodeAssign(nodesDestination,nodesSource,source->nodeCount,move);
		break;
	}
	case sizeof(uint32_t)*8:{
		const Node32* nodesSource=(const Node32*)(source->nodes);
		NodeAssign(nodesDestination,nodesSource,source->nodeCount,move);
		break;
	}
	case sizeof(uint64_t)*8:{
		const Node64* nodesSource=(const Node64*)(source->nodes);
		NodeAssign(nodesDestination,nodesSource,source->nodeCount,move);
		break;
	}
	default:
		return false;
	}
	TreeInformationAssign(destination,source);
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
template<typename AnotherRBTreeArrayType>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::Transform(const AnotherRBTreeArrayType& another){
	CheckTransformable(another);
	if(another.ArraySize()<=ArraySize()){
		Assign(tree,another.Data());
		return true;
	}else{
		if(another.ArraySize()<MaxNodeCount){
			RBTree* newTree=CreateSize(another.ArraySize());
			if(!newTree){
				return false;
			}
			Assign(newTree,another.Data());
			this->~RBTreeArray();
			tree=newTree;
			return true;
		}else{
			return false;
		}
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::SetTree(RBTree* another){
	if(another->bitLength!=bitLength){
		return false;
	}
	if(another==tree){
		return false;
	}
	this->~RBTreeArray();
	tree=another;
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::SetTreeWithoutDestoryMyTree(RBTree* another){
	if(another->bitLength!=bitLength){
		return false;
	}
	tree=another;
	return true;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline void RBTreeArray<KeyType,ValueType,IndexType,BitLength>::CheckColor(){
	printf("=== Checking Color ===\n");
	Node* nodes=(Node*)(tree->nodes);
	for(uint64_t index=0;index<KeyCount();index=index+1){
		if(nodes[index].color!=static_cast<uint32_t>(Color::Red)&&nodes[index].color!=static_cast<uint32_t>(Color::Black)){
			throw std::invalid_argument("RBTreeArray: invalid color");
		}
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::operator=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another){
	CheckAssignable(another); // no use
	if(this!=&another){
		Transform(another);
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline RBTreeArray<KeyType,ValueType,IndexType,BitLength>& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::operator=(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another){
	CheckAssignable(another); // no use
	if(this!=&another){
		SetTree(another.Data());
		RBTree* newTree=CreateSize(0);
		another.SetTreeWithoutDestoryMyTree(newTree);
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline IndexType RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMinIndex(RBTree* tree){
	if(tree&&tree->nodeCount){
		Node* nodes=(Node*)(tree->nodes);
		Node* current=nodes+tree->rootIndex;
		while(current->leftIndex!=MaxNodeCount){
			current=nodes+current->leftIndex;
		}
		return current-nodes;
	}
	return MaxNodeCount;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline IndexType RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMaxIndex(RBTree* tree){
	if(tree&&tree->nodeCount){
		Node* nodes=(Node*)(tree->nodes);
		Node* current=nodes+tree->rootIndex;
		while(current->rightIndex!=MaxNodeCount){
			current=nodes+current->rightIndex;
		}
		return current-nodes;
	}
	return MaxNodeCount;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline IndexType RBTreeArray<KeyType,ValueType,IndexType,BitLength>::IndexSmallestGraterThan(const KeyType& key)const noexcept{
	if(!KeyCount()){
		return MaxNodeCount;
	}
	Node* nodes=(Node*)(tree->nodes);
	IndexType candidate=MaxNodeCount;
	Node* current=nodes+tree->rootIndex;
	while(true){
		if(key<current->key){
			candidate=current-nodes;
			if(current->leftIndex==MaxNodeCount){
				break;
			}
			current=nodes+current->leftIndex;
		}else{
			if(current->rightIndex==MaxNodeCount){
				break;
			}
			current=nodes+current->rightIndex;
		}
	}
	return candidate;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline IndexType RBTreeArray<KeyType,ValueType,IndexType,BitLength>::IndexBiggestSmallerThan(const KeyType& key)const noexcept{
	if(!KeyCount()){
		return MaxNodeCount;
	}
	Node* nodes=(Node*)(tree->nodes);
	IndexType candidate=MaxNodeCount;
	Node* current=nodes+tree->rootIndex;
	while(true){
		if(key>current->key){
			candidate=current-nodes;
			if(current->rightIndex==MaxNodeCount){
				break;
			}
			current=nodes+current->rightIndex;
		}else{
			if(current->leftIndex==MaxNodeCount){
				break;
			}
			current=nodes+current->leftIndex;
		}
	}
	return candidate;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetSmallestGraterThan(const KeyType& key,KeyType& greater,ValueType& value)const noexcept{
	IndexType index=IndexSmallestGraterThan(key);
	if(index!=MaxNodeCount){
		Node* nodes=(Node*)(tree->nodes);
		greater=nodes[index].key;
		value=nodes[index].value;
		return true;
	}
	return false;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetBiggestSmallerThan(const KeyType& key,KeyType& smaller,ValueType& value)const noexcept{
	IndexType index=IndexBiggestSmallerThan(key);
	if(index!=MaxNodeCount){
		Node* nodes=(Node*)(tree->nodes);
		smaller=nodes[index].key;
		value=nodes[index].value;
		return true;
	}
	return false;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::begin()const{
	if(!tree){
		return end();
	}
	if(tree->nodeCount==0){
		return end();
	}
	return UnorderedIterator(tree,0);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::end()const{
	return UnorderedIterator(tree,tree->nodeCount);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedBegin()const{
	if(!tree){
		return OrderedEnd();
	}
	if(tree->nodeCount==0){
		return OrderedEnd();
	}
	IndexType minIndex=GetMinIndex(tree);
	return OrderedIterator(tree,minIndex);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedEnd()const{
	return OrderedIterator(tree,MaxNodeCount,false,true);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline const KeyType& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::Key(){
	Node* nodes=(Node*)(tree->nodes);
	return nodes[currentIndex].key;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline ValueType& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::Value(){
	Node* nodes=(Node*)(tree->nodes);
	return nodes[currentIndex].value;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator++(){
	if(tree&&tree->nodeCount){
		if(reachedBegin){
			currentIndex=RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMinIndex(tree);
			reachedBegin=false;
			return *(this);
		}
		if(currentIndex!=MaxNodeCount){
			Node* nodes=(Node*)(tree->nodes);
			Node* current=nodes+currentIndex;
			if(current->rightIndex!=MaxNodeCount){
				current=nodes+current->rightIndex;
				while(current->leftIndex!=MaxNodeCount){
					current=nodes+current->leftIndex;
				}
				currentIndex=current-nodes;
			}else{
				while(true){
					if(current->fatherIndex==MaxNodeCount){
						reachedEnd=true;
						currentIndex=MaxNodeCount;
						break;
					}
					if((nodes+current->fatherIndex)->rightIndex!=current-nodes){
						currentIndex=current->fatherIndex;
						break;
					}
					current=nodes+current->fatherIndex;
				}
			}
		}
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator++(int){
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator before=*(this);
	++*(this);
	return before;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator--(){
	if(tree&&tree->nodeCount){
		if(reachedEnd){
			currentIndex=RBTreeArray<KeyType,ValueType,IndexType,BitLength>::GetMaxIndex(tree);
			reachedEnd=false;
			return *(this);
		}
		if(currentIndex!=MaxNodeCount){
			Node* nodes=(Node*)(tree->nodes);
			Node* current=nodes+currentIndex;
			if(current->leftIndex!=MaxNodeCount){
				current=nodes+current->leftIndex;
				while(current->rightIndex!=MaxNodeCount){
					current=nodes+current->rightIndex;
				}
				currentIndex=current-nodes;
			}else{
				while(true){
					if(current->fatherIndex==MaxNodeCount){
						reachedBegin=true;
						currentIndex=MaxNodeCount;
						break;
					}
					if((nodes+current->fatherIndex)->leftIndex!=current-nodes){
						currentIndex=current->fatherIndex;
						break;
					}
					current=nodes+current->fatherIndex;
				}
			}
		}
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator--(int){
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator before=*(this);
	--*(this);
	return before;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator==(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator& another)const{
	return another.tree==tree&&another.currentIndex==currentIndex&&another.reachedBegin==reachedBegin&&another.reachedEnd==reachedEnd;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator::operator!=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>::OrderedIterator& another)const{
	return !(*(this)==another);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedBegin()const{
	if(!tree){
		return UnorderedEnd();
	}
	if(tree->nodeCount==0){
		return UnorderedEnd();
	}
	return UnorderedIterator(tree,0);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedEnd()const{
	if(tree){
		return UnorderedIterator(tree,tree->nodeCount);
	}
	return UnorderedIterator(tree,0);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline const KeyType& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::Key(){
	Node* nodes=(Node*)(tree->nodes);
	return nodes[currentIndex].key;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline ValueType& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::Value(){
	Node* nodes=(Node*)(tree->nodes);
	return nodes[currentIndex].value;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator++(){
	if(tree&&tree->nodeCount){
		if(currentIndex<tree->nodeCount||currentIndex==-1){
			currentIndex=currentIndex+1;
		}
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator++(int){
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator before=*(this);
	++*(this);
	return before;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator& RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator--(){
	if(tree&&tree->nodeCount){
		if(currentIndex>0){
			currentIndex=currentIndex-1;
		}else{
			reachedBegin=true;
		}
	}
	return *(this);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator--(int){
	RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator before=*(this);
	--*(this);
	return before;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator+(long long gap)const{
	if(gap<0){
		return *(this)-(-gap);
	}
	if(currentIndex+gap>=tree->nodeCount){
		return RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator(tree,tree->nodeCount);
	}else{
		return RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator(tree,currentIndex+gap);
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline typename RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator-(long long gap)const{
	if(gap<0){
		return *(this)+(-gap);
	}
	if((long long)currentIndex-gap<0){
		return RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator(tree,MaxNodeCount,true);
	}else{
		return RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator(tree,currentIndex-gap);
	}
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator==(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator& another)const{
	return another.tree==tree&&another.currentIndex==currentIndex;
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline bool RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator!=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator& another)const{
	return !(*(this)==another);
}

template<typename KeyType,typename ValueType,typename IndexType,unsigned BitLength>
inline std::pair<const KeyType&,ValueType&> RBTreeArray<KeyType,ValueType,IndexType,BitLength>::UnorderedIterator::operator*()const{
	Node* nodes=(Node*)(tree->nodes);
	return {nodes[currentIndex].key,nodes[currentIndex].value};
}

#endif