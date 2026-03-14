/*LRU要求:存 key-value
         有固定容量,满了就删掉最久没使用的那个
         支持两个操作：get(key)、put(key, value)*/

#include <iostream>
#include <unordered_map>//哈希表:查找快
#include <list>//双向链表：顺序清晰
using namespace std;
 
//双向链表节点
struct Node{
    //结构信息
    int key;
    int value;
    Node* prev;
    Node* next;

    //初始化：构造函数
    Node(int k,int v):key(k),value(v),prev(nullptr),next(nullptr){}
};

//LRUCache类
class LRUCache{
private:
    //双向链表的虚拟头节点，尾节点
    Node* head;
    Node* tail;
    //哈希表
    unordered_map<int,Node*> cache;
    //容量
    int cap;

    //删除任意节点
    void removeNode(Node* node);
    //最近使用插入头部
    void addToHead(Node* node);
    //访问过的的移动到头部
    void moveToHead(Node* node);
    //删除尾部最久未使用的节点
    void removeTail();

public:
    //构造函数
    LRUCache(int capacity){
        cap=capacity;
        head=new Node(-1,-1);
        tail=new Node(-1,-1);
        head->next=tail;
        tail->next=head;
    }
    //获取数据
    int get(int key);
    //更新数据
    void put(int kry,int value);
};

//删除指定节点
void LRUCache::removeNode(Node* node){
    Node* cur=node->next;
    cur->prev=node->prev;
    node->prev->next=cur;
}

//插到头节点后面
void LRUCache::addToHead(Node* node){
    node->next=head->next;
    node->prev=head;
    head->next=node;
    head->next->prev=node;
}

//先删除，再插入头部
void LRUCache::moveToHead(Node* node){
    removeNode(node);
    addToHead(node);
}

//删除尾部节点
void LRUCache::removeTail(){
    Node* delNode=tail->prev;//尾节点
    removeNode(delNode);//从链表移除
    cache.erase(delNode->key);//从哈希表删除
    delete delNode;//释放内存
}

//实现get函数
/*查哈希表，找不到返回 -1
  找到 → 把节点移到头部
  返回对应的 value*/
int LRUCache::get(int key){
    //在哈希表中没有找到key
    if(cache.find(key)==cache.end()){
        return -1;
    }
    //找到
    Node* node=cache[key];

    //放到头部
    moveToHead(node);

    //返回值
    return node->value;
}

//实现put函数
/*如果 key 已存在 → 更新值，并移到头部
  如果 key 不存在 → 创建新节点，加到头部
  加入后超出容量 → 删除尾节点（最久未使用），同时删除哈希表对应项
  记得维护哈希表*/
void LRUCache::put(int key,int value){
    //存在:值更新+到头部
    if (cache.find(key)!=cache.end()){
        Node* node=cache[key];
        node->value=value;
        moveToHead(node);
        return;
    }
    //不存在：建立新节点
    Node* newNode=new Node(key,value);
    cache[key]=newNode;//加入哈希表
    addToHead(newNode);//加入链表头部
    //判断是否超出容量
    if(cache.size()>cap){
        Node* delNode=tail->prev;
        removeNode(delNode);//删除久久未使用的节点
        cache.erase(delNode->key);//删除哈希表
        delete delNode;//释放内存
    }
}

//主程序入口
int main(){
    LRUCache lru(2);

    lru.put(1,10);
    lru.put(2,20);

    cout<<lru.get(1)<<endl;

    lru.put(3,30);
    cout<<lru.get(2)<<endl;

    lru.put(4,40);
    cout<<lru.get(1)<<endl;
    cout<<lru.get(3)<<endl;
    cout<<lru.get(4)<<endl;

    return 0;
}