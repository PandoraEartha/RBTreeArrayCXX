# Usage

`#include "RBTreeArrayCXX.h"`

# RBTreeArrayCXX
Red black tree C++ implementation, tree and nodes are in a continuous memory region thus you can write the whole tree into file/shared_memory or read from file/shared_memory  

# Public Interface Details:

## `RBTreeArray();`
Default constructor, creat RBTreeArray with default array size(256), 
Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree32;
RBTreeArray16<double,unsigned> tree16;
```

## `RBTreeArray(uint64_t size);`
Constructor, creat RBTreeArray with specific size
Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree32(100000);
RBTreeArray16<double,unsigned> tree16(65535);
```
If size >= the most size that the tree allowed, it will create RBTreeArray with the most size that the tree allowed

## `RBTreeArray(std::initializer_list<std::pair<KeyType,ValueType>> initList);`
Constructor, creat RBTreeArray with with default array size(256) and std::initializer_list
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
```
    
## `RBTreeArray(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);`
Constructor, creat RBTreeArray by copying from another RBTreeArray
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
RBTreeArray32<unsigned,double> tree32Copy=tree32; // tree32: {{1,2},{3,4},{5,6}}, tree32Copy: {{1,2},{3,4},{5,6}}
```

## `RBTreeArray(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);`
Constructor, creat RBTreeArray by stealing from another RBTreeArray
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
RBTreeArray32<unsigned,double> tree32Copy=tree32; // tree32: {{,}}, tree32Copy: {{1,2},{3,4},{5,6}}
```

## `~RBTreeArray();`
Destructor

## `bool Insert(const KeyType& key,const ValueType& value)noexcept;`
Insert a key and its corresponding value
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32;
tree32.Insert(3,3.1415926);
```
Return true if the key was successfully inserted or already existed, if the key already existed, its value will be replace
Return false if the key count of the tree has hit the maximum of the array size and the key dose not existed in the tree

## `bool Delete(const KeyType& key)noexcept;`
Delete a key-value pair form the tree
Usage example: 
```C++
tree.Delete();
```
Return true if key existed in the tree, false if key dose not existed in the tree

## `uint64_t ConditionalDelete(ConditionFunction&& condition,Parameters&&... parameters);`
Delete all key-value pairs that condition returns true, condition can be function pointer, std::function, lambda. parameters can be any type
condition must receive at least key and value
Return the number of key-value pairs deleted

## `uint64_t ConditionalDeleteOnce(ConditionFunction&& condition,Parameters&&... parameters)noexcept;`
Delete one key-value pair that condition returns true, condition can be function pointer, std::function, lambda. parameters can be any type
condition must receive at least key and value
Return the number of key-value pairs deleted
If condition returns true on more than one key-value pairs, it is not guarantee that the delete is the minimum

## `bool Search(const KeyType& key,ValueType& value)const noexcept;`
Receive key and value, Search by key in tree, and store its corresponding value into value
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32;
// ...
unsigned key=3;
double value;
tree32.Search(key,value); // searched value store in value
```
Return true if key existed in tree

## `bool GetMin(KeyType& key,ValueType& value)const noexcept;`
Get the minimum key and its corresponding value
Return true if there is at least one key in tree

## `bool GetMax(KeyType& key,ValueType& value)const noexcept;`
Get the maximum key and its corresponding value
Return true if there is at least one key in tree

## `bool GetSmallestGraterThan(const KeyType& key,KeyType& greater,ValueType& value)const noexcept;`
Get the smallest key and its corresponding value that greater than the giving key
Return true if exist

## `bool GetBiggestSmallerThan(const KeyType& key,KeyType& smaller,ValueType& value)const noexcept;`
Get the biggest key and its corresponding value that smaller than the giving key
Return true if exist

## `std::vector<KeyType> Keys()const;`
Get all keys

## `std::vector<ValueType> Values()const;`
Get all values

## `std::vector<std::pair<KeyType,ValueType>> KeysValues()const;`
Get all key-value pairs

## `std::vector<const KeyType*> KeysPointer()const;`
Get pointers of all keys
Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.

## `std::vector<ValueType*> ValuesPointer()const;`
Get pointers of all values
Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.

## `std::vector<std::pair<const KeyType*,ValueType*>> KeysValuesPointer()const;`
Get pointers of all key-value pairs
Warning: pointers will be invalid once the tree has changed, including inserting, deleteing, resize, etc.

## `bool MemoryShrink()noexcept;`
Shrink the array size to the key count of the tree
return true if malloc success or key count == current array size

## `bool ReSize(uint64_t size);`
Resize the array size
return true if malloc success or size == current array size

## `void Clear();`
Set tree to empty tree, will not release the memory
Call Clear() first than MemoryShrink() to release the memory use

## `bool IsEmpty();`
Return true if tree is empty

## `RBTreeData()const{return tree;}`
Return a C style pointer that point to the RBTree struct, call ByteSize() to get tha Byte size of the struct

## `bool SetTree(RBTreeanother);`
Set this tree from another RBTree struct pointer, the bit length of this tree and another must be the same
Warning: The key type and value type of this tree and another must be the same, or it will be undefined behavior
Return true if the bit length is same
Warning: After calling this function, my previous tree will be destoryed

## `bool SetTreeWithoutDestoryMyTree(RBTreeanother);`
Set this tree from another RBTree struct pointer without destory my tree, the bit length of this tree and another must be the same
Warning: The key type and value type of this tree and another must be the same, or it will be undefined behavior
Return true if the bit length is same

## `uint64_t KeyCount()const;`
Return the key-value pair count

## `uint64_t ArraySize()const;`
Return the node array size of this RBTreeArray, greater or equal to the key count

## `uint64_t GetBitLength()const;`
Return the bit length

## `uint64_t SizeAvailable()const;`
Return the maximum number of key-value pair that can be inserted

## `bool Transform(const AnotherRBTreeArrayType& another);`
Transform the data from another tree with different bit length, after calling this function, this tree and another will have the same key-value data with different bit length
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}}; // tree32: {{1,2},{3,4},{5,6}}
RBTreeArray16<unsigned,double> tree16;                     // tree16: {,}
tree16.Transform(tree32)                                   // tree16: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
// tree16 and tree32 now have the same key-value pairs but different bit length
```
Return false if the KeyCount of another tree is greater than the maximum node number that this tree allowed or malloc failed

## `ValueType& operator[](const KeyType& key);`
Return the reference of the value paired to the key
If the key does not exist, it will creat a node with the giving key
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
tree32[7]=3.1415926;
tree32[7]=2*3.1415926;
```
throw std::out_of_range("RBTreeArray: Both search and insert failed when using operator []") if both search and insert have failed

## `RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(const RBTreeArray<KeyType,ValueType,IndexType,BitLength>& another);`
operator =
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
RBTreeArray32<unsigned,double> tree32Copy;
tree32Copy=tree32; // tree32Copy: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
```
    
## `RBTreeArray<KeyType,ValueType,IndexType,BitLength>& operator=(RBTreeArray<KeyType,ValueType,IndexType,BitLength>&& another);`
operator = with move copy
Usage example: 
```C++
RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
RBTreeArray32<unsigned,double> tree32Steal;
tree32Steal=std::move(tree32); // tree32Steal: {{1,2},{3,4},{5,6}}, tree32: {{1,2},{3,4},{5,6}}
```

# Iterator:

## UnorderedIterator:
Iterator of the array order, this kind of traversal is much faster but unordered 

### `UnorderedIterator begin();`
Return UnorderedIterator at the begin of UnorderedIterator

### `UnorderedIterator end();  
Return UnorderedIterator at the end of UnorderedIterator

### Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree;
// ...
for(auto iterator=tree.begin();iterator!=tree.end();++iterator){
    auto key=iterator.Key();
    suto value=iterator.Value();
}
for(auto iterator=tree.begin();iterator!=tree.end();iterator++){
    // ...
}
for(auto iterator=tree.begin();iterator!=tree.end();iterator=iterator+1){
    // ...
}
for(const auto& [key,value]:tree){ // need C++17
    // ...
}
```

### `UnorderedIterator UnorderedBegin();`
Equals to begin()

### `UnorderedIterator UnorderedEnd();`
Equals to end()

### `const KeyType& UnorderedIterator::Key();`
### `ValueType& UnorderedIterator::Value();`
Get key or value reference from UnorderedIterator
Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree;
// ...
for(auto iterator=tree.begin();iterator!=tree.end();++iterator){
    auto key=iterator.Key();
    suto value=iterator.Value();
}
```

## OrderedIterator:
Iterator of the key order, this kind of traversal is slower than UnorderedIterator but ordered

### `OrderedIterator OrderedBegin();`
Return OrderedIterator at the begin of OrderedIterator

### `OrderedIterator OrderedEnd();`
Return OrderedIterator at the end of OrderedIterator

### Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree;
// ...
for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();++iterator){
    auto key=iterator.Key();
    suto value=iterator.Value();
}
for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();iterator++){
    // ...
}
```

### `const KeyType& OrderedIterator::Key();`
### `ValueType& OrderedIterator::Value();`
Get key or value reference from OrderedIterator
Usage example: 
```C++
RBTreeArray32<std::string,std::vector<double>> tree;
// ...
for(auto iterator=tree.OrderedBegin();iterator!=tree.OrderedEnd();++iterator){
    auto key=iterator.Key();
    suto value=iterator.Value();
}
```
