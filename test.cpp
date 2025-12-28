#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <chrono>
#include <cassert>

// 包含你的随机引擎头文件
#include "PCG32.h"

// 包含你的RBTreeArray头文件
#include "RBTreeArrayCXX.h"

using namespace std;

class TestRBTreeArray {
private:
    PCG32Struct rng;
    
public:
    TestRBTreeArray() {
        PCG32SetSeed(&rng, time(NULL));
    }
    
    // 基础功能测试
    template<typename RBTreeType>
    void testBasicOperations() {
        cout << "Testing basic operations..." << endl;
        
        RBTreeType tree;
        map<int, string> stdMap;
        
        // 插入测试
        for (int i = 0; i < 1000; ++i) {
            int key = PCG32Uniform(&rng, 0, 10000);
            string value = "value_" + to_string(key);
            
            tree.Insert(key, value);
            stdMap[key] = value;
        }
        
        // 搜索测试
        for (const auto& pair : stdMap) {
            string value;
            bool found = tree.Search(pair.first, value);
            assert(found && "Key should be found");
            assert(value == pair.second && "Value should match");
        }
        
        // 最小最大值测试
        if (!stdMap.empty()) {
            int minKey, maxKey;
            string minValue, maxValue;
            
            tree.GetMin(minKey, minValue);
            tree.GetMax(maxKey, maxValue);
            
            auto stdMin = stdMap.begin();
            auto stdMax = stdMap.rbegin();
            
            assert(minKey == stdMin->first && "Min key should match");
            assert(maxKey == stdMax->first && "Max key should match");
        }
        
        cout << "Basic operations test passed!" << endl;
    }
    
    // 删除测试
    template<typename RBTreeType>
    void testDeletion() {
        cout << "Testing deletion..." << endl;
        
        RBTreeType tree;
        map<int, int> stdMap;
        
        // 插入一些数据
        for (int i = 0; i < 500; ++i) {
            int key = PCG32Uniform(&rng, 0, 1000);
            tree.Insert(key, key * 2);
            stdMap[key] = key * 2;
        }
        
        // 随机删除一半的数据
        vector<int> keysToDelete;
        for (const auto& pair : stdMap) {
            if (PCG32Uniform(&rng, 0, 2) == 0) {
                keysToDelete.push_back(pair.first);
            }
        }
        
        for (int key : keysToDelete) {
            tree.Delete(key);
            stdMap.erase(key);
        }
        
        // 验证剩余数据
        for (const auto& pair : stdMap) {
            int value;
            bool found = tree.Search(pair.first, value);
            assert(found && "Remaining key should be found");
            assert(value == pair.second && "Remaining value should match");
        }
        
        // 验证已删除的数据
        for (int key : keysToDelete) {
            int value;
            bool found = tree.Search(key, value);
            assert(!found && "Deleted key should not be found");
        }
        
        cout << "Deletion test passed!" << endl;
    }
    
    // 性能对比测试
    template<typename RBTreeType>
    void testPerformance() {
        cout << "Testing performance vs std::map..." << endl;
        
        RBTreeType tree;
        map<int, int> stdMap;
        
        const int OPERATIONS = 10000;
        
        // 插入性能测试
        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < OPERATIONS; ++i) {
            int key = PCG32Uniform(&rng, 0, OPERATIONS * 2);
            tree.Insert(key, i);
        }
        auto treeInsertTime = chrono::high_resolution_clock::now() - start;
        
        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < OPERATIONS; ++i) {
            int key = PCG32Uniform(&rng, 0, OPERATIONS * 2);
            stdMap[key] = i;
        }
        auto mapInsertTime = chrono::high_resolution_clock::now() - start;
        
        // 搜索性能测试
        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < OPERATIONS; ++i) {
            int key = PCG32Uniform(&rng, 0, OPERATIONS * 2);
            int value;
            tree.Search(key, value);
        }
        auto treeSearchTime = chrono::high_resolution_clock::now() - start;
        
        start = chrono::high_resolution_clock::now();
        for (int i = 0; i < OPERATIONS; ++i) {
            int key = PCG32Uniform(&rng, 0, OPERATIONS * 2);
            auto it = stdMap.find(key);
        }
        auto mapSearchTime = chrono::high_resolution_clock::now() - start;
        
        cout << "Insert - RBTree: " 
             << chrono::duration_cast<chrono::microseconds>(treeInsertTime).count() 
             << "us, std::map: "
             << chrono::duration_cast<chrono::microseconds>(mapInsertTime).count() 
             << "us" << endl;
        
        cout << "Search - RBTree: " 
             << chrono::duration_cast<chrono::microseconds>(treeSearchTime).count() 
             << "us, std::map: "
             << chrono::duration_cast<chrono::microseconds>(mapSearchTime).count() 
             << "us" << endl;
    }
    
    // Transform 测试
    void testTransform() {
        cout << "Testing transform between different RBTreeArray types..." << endl;
        
        // 创建源树 (16位)
        RBTreeArray16<int, string> sourceTree;
        for (int i = 0; i < 100; ++i) {
            sourceTree.Insert(i, "value_" + to_string(i));
        }
        
        // 转换到32位
        RBTreeArray32<int, string> targetTree32;
        bool success32 = targetTree32.Transform(sourceTree);
        assert(success32 && "Transform to 32-bit should succeed");
        
        // 验证数据
        for (int i = 0; i < 100; ++i) {
            string value;
            bool found = targetTree32.Search(i, value);
            assert(found && "Key should be found after transform");
            assert(value == "value_" + to_string(i) && "Value should match after transform");
        }
        
        // 转换到64位
        RBTreeArray64<int, string> targetTree64;
        bool success64 = targetTree64.Transform(targetTree32);
        assert(success64 && "Transform to 64-bit should succeed");
        
        // 验证数据
        for (int i = 0; i < 100; ++i) {
            string value;
            bool found = targetTree64.Search(i, value);
            assert(found && "Key should be found after transform");
            assert(value == "value_" + to_string(i) && "Value should match after transform");
        }
        
        cout << "Transform test passed!" << endl;
    }
    
    // 边界条件测试
    template<typename RBTreeType>
    void testEdgeCases() {
        cout << "Testing edge cases..." << endl;
        
        RBTreeType tree;
        
        // 空树操作
        int key, value;
        assert(!tree.GetMin(key, value) && "GetMin should fail on empty tree");
        assert(!tree.GetMax(key, value) && "GetMax should fail on empty tree");
        assert(!tree.Delete(1) && "Delete should fail on empty tree");
        
        // 单个元素
        tree.Insert(1, 100);
        assert(tree.GetMin(key, value) && key == 1 && value == 100);
        assert(tree.GetMax(key, value) && key == 1 && value == 100);
        
        // 重复插入
        tree.Insert(1, 200);
        tree.Search(1, value);
        assert(value == 200 && "Value should be updated on duplicate insert");
        
        // 删除唯一元素
        assert(tree.Delete(1) && "Delete should succeed");
        assert(!tree.Search(1, value) && "Search should fail after deletion");
        
        cout << "Edge cases test passed!" << endl;
    }
    
    // 运行所有测试
    void runAllTests() {
        cout << "=== Testing RBTreeArray16 ===" << endl;
        testBasicOperations<RBTreeArray16<int, string>>();
        testDeletion<RBTreeArray16<int, int>>();
        testPerformance<RBTreeArray16<int, int>>();
        testEdgeCases<RBTreeArray16<int, int>>();
        
        cout << "\n=== Testing RBTreeArray32 ===" << endl;
        testBasicOperations<RBTreeArray32<int, string>>();
        testDeletion<RBTreeArray32<int, int>>();
        testPerformance<RBTreeArray32<int, int>>();
        testEdgeCases<RBTreeArray32<int, int>>();
        
        cout << "\n=== Testing RBTreeArray64 ===" << endl;
        testBasicOperations<RBTreeArray64<int, string>>();
        testDeletion<RBTreeArray64<int, int>>();
        testPerformance<RBTreeArray64<int, int>>();
        testEdgeCases<RBTreeArray64<int, int>>();
        
        cout << "\n=== Testing Transform ===" << endl;
        testTransform();
        
        cout << "\n=== All tests passed! ===" << endl;
    }
};

#include <functional>

template<typename Iterator>
std::function<void(Iterator&)>IteratorNext(){
    return [](Iterator& iterator){iterator++;};
}

template<typename Iterator>
std::function<void(Iterator&)>IteratorPrivious(){
    return [](Iterator& iterator){iterator--;};
}

void IteratorTest(){
    const unsigned size=20;
    PCG32Struct PCG32Status;
    PCG32SetSeed(&PCG32Status,time(NULL));
    RBTreeArray16<unsigned,std::string> tree16;
    std::map<unsigned,std::string> map16;
    for(unsigned index=0;index<size;index=index+1){
        tree16.Insert(index,std::to_string(index));
        map16[index]=std::to_string(index);
    }
    if(true){
        for(auto iterator=map16.begin();iterator!=map16.end();iterator++){
            printf("%u, %s\n",iterator->first,iterator->second.c_str());
        }
        for(auto iterator=tree16.begin();iterator!=tree16.end();iterator++){
            printf("%u, %s\n",iterator.Key(),iterator.Value().c_str());
        }
    }
    if(true){
        printf("===============\n");
        auto iterator=tree16.end();
        while(true){
            iterator--;
            printf("%u, %s\n",iterator.Key(),iterator.Value().c_str());
            if(iterator==tree16.begin()){
                break;
            }
        }
    }
    if(true){
        printf("===============\n");
        auto iterator=tree16.begin();
        std::function<void(RBTreeArray16<unsigned,std::string>::UnorderedIterator&)> Operation=IteratorNext<RBTreeArray16<unsigned,std::string>::UnorderedIterator>();
        unsigned count=0;
        while(count<100){
            printf("%u, %s\n",iterator.Key(),iterator.Value().c_str());
            Operation(iterator);
            count=count+1;
            if(iterator==tree16.end()){
                Operation=IteratorPrivious<RBTreeArray16<unsigned,std::string>::UnorderedIterator>();
                Operation(iterator);
                continue;
            }
            if(iterator==tree16.begin()){
                printf("%u, %s\n",iterator.Key(),iterator.Value().c_str());
                Operation(iterator);
                Operation=IteratorNext<RBTreeArray16<unsigned,std::string>::UnorderedIterator>();
                Operation(iterator);
            }
        }
    }
    if(true){
        RBTreeArray64<unsigned,std::string> tree64;
        for(auto iterator=tree64.begin();iterator!=tree64.end();iterator++){
            printf("EMPTY TREE\n!");
        }
    }
    if(true){
        RBTreeArray64<unsigned,double> tree64={{1,2},{3,4},{5,6}};
        for(auto iterator=tree64.begin();iterator!=tree64.end();iterator++){
            printf("%u, %lf\n",iterator.Key(),iterator.Value());
        }
    }
    if(true){
        RBTreeArray64<unsigned,double> tree64={{1,2},{3,4},{5,6}};
        for(unsigned index=0;index<64;index=index+1){
            const double pi=3.1415926535897932384626433832795;
            tree64[index]=index*pi;
        }
        for(auto iterator=tree64.UnorderedBegin();iterator!=tree64.UnorderedEnd();iterator++){
            printf("%u, %lf\n",iterator.Key(),iterator.Value());
        }
    }
}

static inline bool IfDelete_10Outof1(unsigned key,unsigned value){
    // return false;
    // return key%101==0;
    // return true;
    // return key%2==0;
    // return key%4==0;
    return key%10==0;
}

static inline bool IfDelete(unsigned key,unsigned value){
    // return false;
    // return key%101==0;
    // return true;
    // return key%2==0;
    // return key%4==0;
    return key%3==0;
}

static inline bool IfDeleteRandom(const std::string& key,const std::vector<std::string>& value,PCG32Struct* PCGStatus){
    return stoi(key)%11==0;
    return true;
    return PCG32Uniform_Strict(PCGStatus,0,10)<0;
}

template<typename RBTreeArray,typename Map>
bool NodeCompare(const RBTreeArray& tree,const Map& map){
    if(tree.KeyCount()!=map.size()){
        return false;
    }
    auto OrderedIterator=tree.OrderedBegin();
    auto mapIterator=map.begin();
    while(OrderedIterator!=tree.OrderedEnd()){
        if(OrderedIterator.Key()!=mapIterator->first){
            return false;
        }
        ++mapIterator;
        ++OrderedIterator;
    }
    return true;
}

#include <sys/time.h>

template<typename RBTreeArray,typename Map>
void SpeedTestTemplate(RBTreeArray& tree,Map& map,const long long unsigned int Case){

    PCG32Struct PCGStatus;
    PCG32SetSeed(&PCGStatus,time(NULL));

    unsigned* keys=(unsigned*)malloc(sizeof(unsigned)*Case);
    unsigned* UnorderedKeysCopy=(unsigned*)malloc(sizeof(unsigned)*Case);
    for(long long unsigned int index=0;index<Case;index=index+1){
        keys[index]=PCG32(&PCGStatus);
    }
    memcpy(UnorderedKeysCopy,(const unsigned*)keys,sizeof(unsigned)*Case);
    PCG32UniformShuffle(&PCGStatus,UnorderedKeysCopy,Case);

    struct timeval start,end;
    unsigned millisecondsRBTreeArray=0;
    unsigned millisecondsStdmap=0;

    gettimeofday(&start,NULL);
    for(long long unsigned int index=0;index<Case;index=index+1){
        tree.Insert(keys[index],index);
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;
    
    gettimeofday(&start,NULL);
    for(long long unsigned int index=0;index<Case;index=index+1){
        map[keys[index]]=index;
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Insert: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

    RBTree* treeCopy=(RBTree*)malloc(tree.ByteSize());
    memcpy(treeCopy,(const RBTree*)tree.Data(),tree.ByteSize());

    gettimeofday(&start,NULL);
    unsigned valueRBTreeArray;
    for(long long unsigned int index=0;index<Case;index=index+1){
        tree.Search(UnorderedKeysCopy[index],valueRBTreeArray);
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    unsigned valueStdmap;
    for(long long unsigned int index=0;index<Case;index=index+1){
        valueStdmap=map.at(UnorderedKeysCopy[index]);
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(valueRBTreeArray!=valueStdmap){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Search: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

    long long unsigned int sum[2]={0LLU};
    gettimeofday(&start,NULL);
    for(auto iterator=tree.UnorderedBegin();iterator!=tree.UnorderedEnd();++iterator){
        sum[0]=sum[0]+iterator.Key();
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(auto iterator=map.begin();iterator!=map.end();++iterator){
        sum[1]=sum[1]+iterator->first;
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(sum[0]!=sum[1]){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Loop Through: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

    gettimeofday(&start,NULL);
    for(long long unsigned int index=0;index<Case;index=index+1){
        tree.Delete(UnorderedKeysCopy[index]);
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(long long unsigned int index=0;index<Case;index=index+1){
        map.erase(UnorderedKeysCopy[index]);
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Delete: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

    tree.SetTree(treeCopy);
    for(long long unsigned int index=0;index<Case;index=index+1){
        map[keys[index]]=index;
    }
    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    auto iteratorArray=tree.OrderedBegin();
    for(auto iterator=map.begin();iterator!=map.end();iterator++){
        if(iteratorArray.Key()!=iterator->first){
            printf("%u, %u\n",iteratorArray.Key(),iterator->first);
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
        iteratorArray++;
    }
    if(iteratorArray!=tree.OrderedEnd()){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    gettimeofday(&start,NULL);
    unsigned deleted=tree.ConditionalDelete(IfDelete);
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(auto iterator=map.begin();iterator!=map.end();){
        if(IfDelete(iterator->first,iterator->second)){
            iterator=map.erase(iterator);
        }else{
            ++iterator;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Conditional Delete 1/3 keys: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

    for(long long unsigned int index=0;index<Case;index=index+1){
        tree[keys[index]]=index;
        map[keys[index]]=index;
    }
    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    gettimeofday(&start,NULL);
    {
        unsigned deleted=tree.ConditionalDelete(IfDelete_10Outof1);
    }
    gettimeofday(&end,NULL);

    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;
    gettimeofday(&start,NULL);
    for(auto iterator=map.begin();iterator!=map.end();){
        if(IfDelete_10Outof1(iterator->first,iterator->second)){
            iterator=map.erase(iterator);
        }else{
            ++iterator;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(!NodeCompare(tree,map)){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }
    printf("  Conditional Delete 1/10 keys: RBTreeArray%u<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",tree.GetBitLength(),millisecondsRBTreeArray,millisecondsStdmap);

}

void SpecialSpeed16(){
    constexpr unsigned scale=128;
    RBTreeArray16<unsigned,unsigned> trees[scale];
    std::map<unsigned,unsigned> maps[scale];
    for(unsigned index=0;index<scale;index=index+1){
        trees[index].ReSize(65535);
    }
    PCG32Struct PCGStatus;
    PCG32SetSeed(&PCGStatus,time(NULL));

    unsigned keys[65535];
    unsigned UnorderedKeysCopy[65535];
    for(long long unsigned int index=0;index<65535;index=index+1){
        keys[index]=PCG32(&PCGStatus);
    }
    memcpy(UnorderedKeysCopy,(const unsigned*)keys,sizeof(unsigned)*65535);
    PCG32UniformShuffle(&PCGStatus,UnorderedKeysCopy,65535);

    struct timeval start,end;
    unsigned millisecondsRBTreeArray=0;
    unsigned millisecondsStdmap=0;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            trees[indexTree].Insert(keys[index],index);
        }
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            maps[indexTree][keys[index]]=index;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    printf("  Insert: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);

    gettimeofday(&start,NULL);
    long long unsigned int searchSum[2]={0};
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            unsigned search;
            trees[indexTree].Search(UnorderedKeysCopy[index],search);
            searchSum[0]=searchSum[0]+search;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            unsigned valueStdmap=maps[indexTree].at(UnorderedKeysCopy[index]);
            searchSum[1]=searchSum[1]+valueStdmap;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(searchSum[0]!=searchSum[1]){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Search: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);

    long long unsigned int sum[2]={0LLU};
    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(auto iterator=trees[indexTree].UnorderedBegin();iterator!=trees[indexTree].UnorderedEnd();++iterator){
            sum[0]=sum[0]+iterator.Key();
        }
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(auto iterator=maps[indexTree].begin();iterator!=maps[indexTree].end();++iterator){
            sum[1]=sum[1]+iterator->first;
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    if(sum[0]!=sum[1]){
        char errorMassage[1024];
        sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
        throw std::logic_error(errorMassage);
    }

    printf("  Loop Through: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(long long unsigned int index=0;index<65535;index=index+1){
            trees[indexTree].Delete(UnorderedKeysCopy[index]);
        }
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(long long unsigned int index=0;index<65535;index=index+1){
            maps[indexTree].erase(UnorderedKeysCopy[index]);
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    printf("  Delete: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);

    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            trees[indexTree].Insert(keys[index],index);
            maps[indexTree][keys[index]]=index;
        }
    }

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        unsigned deleted=trees[indexTree].ConditionalDelete(IfDelete);
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(auto iterator=maps[indexTree].begin();iterator!=maps[indexTree].end();){
            if(IfDelete(iterator->first,iterator->second)){
                iterator=maps[indexTree].erase(iterator);
            }else{
                ++iterator;
            }
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    printf("  Conditional Delete 1/3 keys: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);

    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(unsigned index=0;index<65535;index=index+1){
            trees[indexTree].Insert(keys[index],index);
            maps[indexTree][keys[index]]=index;
        }
    }

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        unsigned deleted=trees[indexTree].ConditionalDelete(IfDelete_10Outof1);
    }
    gettimeofday(&end,NULL);
    millisecondsRBTreeArray=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    gettimeofday(&start,NULL);
    for(unsigned indexTree=0;indexTree<scale;indexTree=indexTree+1){
        for(auto iterator=maps[indexTree].begin();iterator!=maps[indexTree].end();){
            if(IfDelete_10Outof1(iterator->first,iterator->second)){
                iterator=maps[indexTree].erase(iterator);
            }else{
                ++iterator;
            }
        }
    }
    gettimeofday(&end,NULL);
    millisecondsStdmap=(end.tv_sec-start.tv_sec)*1000+(end.tv_usec-start.tv_usec)/1000.0+0.5;

    printf("  Conditional Delete 1/10 keys: RBTreeArray16<unsigned,unsigned>: %u , std::map<unsigned,unsigned>: %u milliseconds\n",millisecondsRBTreeArray,millisecondsStdmap);
}

void SpeedTest(){
    long long unsigned int Case=1234567*2LLU;
    if(false){
        printf("RBTreeArray16:\n");
        RBTreeArray16<unsigned,unsigned> tree16;
        std::map<unsigned,unsigned> map;
        SpeedTestTemplate(tree16,map,65535);
    }else{
        printf("RBTreeArray16:\n");
        SpecialSpeed16();
    }
    if(true){
        printf("RBTreeArray32:\n");
        RBTreeArray32<unsigned,unsigned> tree32;
        std::map<unsigned,unsigned> map;
        SpeedTestTemplate(tree32,map,Case);
    }
    if(true){
        printf("RBTreeArray64:\n");
        RBTreeArray64<unsigned,unsigned> tree64;
        std::map<unsigned,unsigned> map;
        SpeedTestTemplate(tree64,map,Case);
    }
}

void BoundaryTest(){
    if(true){
        PCG32Struct PCGStatus;
        PCG32SetSeed(&PCGStatus,time(NULL));
        unsigned length=PCG32Uniform_Strict(&PCGStatus,10000,50000);

        RBTreeArray64<std::string,std::pair<double,std::vector<std::string>>> tree={{"3.1415926",{3.1415926,{"3",".","1","4"}}}};
        for(unsigned index=0;index<length;index=index+1){
            std::string key=std::to_string(PCG32Uniform_Strict(&PCGStatus,100000,200000));
            std::vector<std::string> vector;
            for(const char c:key){
                vector.push_back({c});
            }
            tree[key]={PCG32UniformReal(&PCGStatus,0,1),vector};
        }
        unsigned treeSize=tree.KeyCount();
        RBTreeArray64<std::string,std::pair<double,std::vector<std::string>>> treeCopy=tree;
        if(treeCopy.KeyCount()!=tree.KeyCount()||tree.KeyCount()!=treeSize){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
        RBTreeArray64<std::string,std::pair<double,std::vector<std::string>>> treeMove=std::move(tree);
        if(tree.KeyCount()!=0||treeMove.KeyCount()!=treeSize){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
    }
    if(true){
        RBTreeArray16<unsigned,unsigned> tree16;
        tree16.ConditionalDelete([](const unsigned key,unsigned value){return (bool)(key&1);});
        tree16[1]=1;
        tree16.ConditionalDelete([](const unsigned key,unsigned value){return (bool)(key&1);});
        tree16.ConditionalDelete([](const unsigned key,unsigned value){return (bool)(key&1);});
        tree16.Delete(0);
        tree16.Delete(1);
        unsigned search;
        tree16.Search(1,search);
        RBTreeArray32<unsigned,unsigned> tree32;
        tree32.Transform(tree16);
        tree16.Transform(tree32);
        tree16[1]=1;
        tree32.Transform(tree16);
        search=0;
        tree32.Search(1,search);
        if(search!=1){
            throw std::logic_error("RBTreeArray error");
        }
        // tree16=tree32;
    }
    if(true){
        PCG32Struct PCGStatus;
        PCG32SetSeed(&PCGStatus,time(NULL));

        RBTreeArray16<std::string,std::vector<std::string>> tree16={{"Hellor",{"H","e","l","l","o"}}};
        std::string string="World!";
        
        for(unsigned index=0;index<10;index=index+1){
            std::vector<std::string> vector;
            for(const char c:string){
                vector.push_back({c});
            }
            tree16.Insert(string,vector);
            string=std::to_string(PCG32Uniform(&PCGStatus,0,999));
        }
        for(auto iterator=tree16.UnorderedBegin();iterator!=tree16.UnorderedEnd();iterator++){
            std::cout<<iterator.Key()<<": ";
            for(const auto iteratorVector:iterator.Value()){
                std::cout<<iteratorVector<<", ";
            }
            printf("\n");
        }
        RBTreeArray32<std::string,std::vector<std::string>> tree32;
        tree32.Transform(tree16);
        for(auto iterator=tree32.UnorderedBegin();iterator!=tree32.UnorderedEnd();iterator++){
            std::cout<<iterator.Key()<<": ";
            for(const auto iteratorVector:iterator.Value()){
                std::cout<<iteratorVector<<", ";
            }
            printf("\n");
        }
        RBTreeArray32<std::string,std::vector<std::string>> tree32Copy;
        tree32Copy=tree32;
    }
    if(true){
        RBTreeArray32<unsigned,double> tree32={{1,2},{3,4},{5,6}};
        for(const auto& [key,value]:tree32){
            printf("%u,%lf\n",key,value);
        }
    }
    if(true){
        PCG32Struct PCGStatus;
        PCG32SetSeed(&PCGStatus,time(NULL));
        std::map<std::string,std::vector<std::string>> map;
        RBTreeArray32<std::string,std::vector<std::string>> tree;
        unsigned length=PCG32Uniform_Strict(&PCGStatus,10000,50000);
        for(unsigned index=0;index<length;index=index+1){
            std::string key=std::to_string(PCG32Uniform_Strict(&PCGStatus,100000,200000));
            std::vector<std::string> vector;
            for(const char c:key){
                vector.push_back({c});
            }
            map[key]=vector;
            tree[key]=vector;
        }
        if(!NodeCompare(tree,map)){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
        RBTreeArray32<std::string,std::vector<std::string>> trees[10];
        trees[0]=std::move(tree);
        tree.Insert("test",{"0","1","2","3"});
        for(unsigned index=1;index<10;index=index+1){
            trees[index]=trees[index-1];
        }
        if(!NodeCompare(trees[9],map)){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
    }
    printf("BoundaryTest passed\n========================\n");
}

void SpecialTestConditionalDelete(){
    PCG32Struct PCGStatus;
    PCG32SetSeed(&PCGStatus,123456789);

    RBTreeArray32<unsigned,unsigned> treeBase;
    RBTreeArray32<unsigned,unsigned> tree;
    std::map<unsigned,unsigned> mapBase;
    std::map<unsigned,unsigned> map;

    long long unsigned int Case=1<<20;
    unsigned* keys=(unsigned*)malloc(sizeof(unsigned)*Case);
    for(long long unsigned int index=0;index<Case;index=index+1){
        keys[index]=PCG32(&PCGStatus);
    }

    for(long long unsigned int index=0;index<Case;index=index+1){
        treeBase[keys[index]]=index;
        mapBase[keys[index]]=index;
    }

    RBTree* treeCopy=(RBTree*)malloc(treeBase.ByteSize());
    memcpy(treeCopy,treeBase.Data(),treeBase.ByteSize());
    long long unsigned int deleted;
    long long unsigned int mapDelete;
    if(false){
        map=mapBase;
        tree.SetTree(treeCopy);
        deleted=tree.ConditionalDelete(IfDelete);
        mapDelete=map.size();
        for(auto iterator=map.begin();iterator!=map.end();){
            if(IfDelete(iterator->first,iterator->second)){
                iterator=map.erase(iterator);
            }else{
                ++iterator;
            }
        }
        mapDelete=mapDelete-map.size();
        if(tree.KeyCount()!=map.size()){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
    }
    if(true){
        std::map<std::string,std::vector<std::string>> map;
        RBTreeArray32<std::string,std::vector<std::string>> tree;
        unsigned length=PCG32Uniform_Strict(&PCGStatus,10000,50000);
        for(unsigned index=0;index<length;index=index+1){
            std::string key=std::to_string(PCG32Uniform_Strict(&PCGStatus,100000,200000));
            std::vector<std::string> vector;
            for(const char c:key){
                vector.push_back({c});
            }
            map[key]=vector;
            tree[key]=vector;
        }
        mapDelete=map.size();
        PCG32Struct PCGStatus0,PCGStatus1;
        PCG32SetSeed(&PCGStatus0,1234567);
        PCG32SetSeed(&PCGStatus1,1234567);
        deleted=tree.ConditionalDelete(IfDeleteRandom,&PCGStatus0);
        for(auto iterator=map.begin();iterator!=map.end();){
            if(IfDeleteRandom(iterator->first,iterator->second,&PCGStatus1)){
                iterator=map.erase(iterator);
            }else{
                ++iterator;
            }
        }
        mapDelete=mapDelete-map.size();
        if(tree.KeyCount()!=map.size()){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
        if(!NodeCompare(tree,map)){
            char errorMassage[1024];
            sprintf(errorMassage,"in %s: %d, RBTreeArray error",__FUNCTION__,__LINE__);
            throw std::logic_error(errorMassage);
        }
    }
    free(keys);
}

#include <sys/resource.h>
#include <unistd.h>

void getDetailedMemoryInfo() {
    // 从 /proc/self/statm 获取详细信息
    FILE *fp = fopen("/proc/self/status", "r");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    char line[256];
    long vmrss_kb = 0;
    long vmhwm_kb = 0;
    long vmsize_kb = 0;
    long vmswap_kb = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        // 当前物理内存使用
        if (strncmp(line, "VmRSS:", 6) == 0) {
            sscanf(line + 6, "%ld", &vmrss_kb);
        }
        // 物理内存峰值
        else if (strncmp(line, "VmHWM:", 6) == 0) {
            sscanf(line + 6, "%ld", &vmhwm_kb);
        }
        // 虚拟内存大小
        else if (strncmp(line, "VmSize:", 7) == 0) {
            sscanf(line + 7, "%ld", &vmsize_kb);
        }
        // SWAP使用量
        else if (strncmp(line, "VmSwap:", 7) == 0) {
            sscanf(line + 7, "%ld", &vmswap_kb);
        }
        // 数据段大小
        else if (strncmp(line, "VmData:", 7) == 0) {
            long vmdata_kb;
            sscanf(line + 7, "%ld", &vmdata_kb);
            // printf("数据段大小: %ld KB\n", vmdata_kb);
        }
        // 栈大小
        else if (strncmp(line, "VmStk:", 6) == 0) {
            long vmstk_kb;
            sscanf(line + 6, "%ld", &vmstk_kb);
            // printf("栈大小: %ld KB\n", vmstk_kb);
        }
    }
    
    fclose(fp);
    
    printf("\n=== 实时内存状态 ===\n");
    printf("当前物理内存(VmRSS): %ld KB (%.2f MB)\n", 
           vmrss_kb, vmrss_kb / 1024.0);
    printf("物理内存峰值(VmHWM): %ld KB (%.2f MB)\n", 
           vmhwm_kb, vmhwm_kb / 1024.0);
    printf("虚拟内存大小(VmSize): %ld KB (%.2f MB)\n", 
           vmsize_kb, vmsize_kb / 1024.0);
    if (vmswap_kb > 0) {
        printf("SWAP使用量(VmSwap): %ld KB\n", vmswap_kb);
    }
}

void MemoryTest(){
    RBTreeArray32<std::string,std::vector<std::string>> tree;
    PCG32Struct PCGStatus;
    PCG32SetSeed(&PCGStatus,time(NULL));
    getDetailedMemoryInfo();

    unsigned insertTimes=1<<23;
    for(unsigned index=0;index<insertTimes;index=index+1){
        tree.Insert(std::to_string(PCG32(&PCGStatus)),{std::to_string(PCG32(&PCGStatus)),std::to_string(PCG32UniformReal(&PCGStatus,0,3.1415926535))});
    }
    tree.MemoryShrink();
    getDetailedMemoryInfo();
    for(unsigned index=0;index<insertTimes;index=index+1){
        if(PCG32(&PCGStatus)&1){
            tree.Insert(std::to_string(PCG32(&PCGStatus)),{std::to_string(PCG32(&PCGStatus)),std::to_string(PCG32UniformReal(&PCGStatus,0,3.1415926535))});
        }else{
            std::string search=std::to_string(PCG32(&PCGStatus));
            std::string toDelete;
            std::vector<std::string> value;
            if(tree.GetBiggestSmallerThan(search,toDelete,value)){

            }else{
                tree.GetSmallestGraterThan(search,toDelete,value);
            }
            tree.Delete(toDelete);
        }
    }
    tree.MemoryShrink();
    getDetailedMemoryInfo();
    tree.Clear();
    tree.MemoryShrink();
    getDetailedMemoryInfo();
}

int main() {
    TestRBTreeArray tester;
    tester.runAllTests();
    
    BoundaryTest();

    IteratorTest();
    SpecialTestConditionalDelete();
    
    SpeedTest();

    // MemoryTest();
    return 0;
}