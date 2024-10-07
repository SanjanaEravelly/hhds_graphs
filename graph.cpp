#include <iostream>
#include <vector>
#include<map>
using namespace std;
enum{
    AND,
    OR,
    NOT
};

class pins{
    public:
    int pid;
    vector<int>edge;
    int portid;
    int getpid(){
        return pid;
    }
    void setpid(int ppid){
        pid=ppid;
    }
    void addPinEdge(int pid){
        edge.push_back(pid);
    }
    void displayPinInfo(){
        cout << "\t\t Edges:";
        for(auto itr=edge.begin(); itr!=edge.end(); ++itr){
            cout<<*itr<<" ";
        }
        cout<<endl;
    }
};
class nodes{
    public:
    int nid;
    vector<int>pin;
    map<int, pins*>pintable;
    int type;
    void addpin(pins* ptr, int pid){
        pin.push_back(pid);
        pintable[pid]=ptr;
    }
    int getid(){
        return nid;
    }
    void setid(int id){
        nid=id;
    }
    /*
    int addnode(int pid, pins* ptr){
        pin.push_back(pid);
    }*/
    void setType(int ty){
        type=ty;
    }    
    int getType(){
        return type;
    }
    void displayNodeInfo(){
            cout<<"Node Info:" << endl;
            cout<<"\t Nid: " << nid <<endl;
            cout<<"\t Type: " << type <<endl;
//            cout<<"\t Pins: ";
            for(auto btr=pintable.begin(); btr!=pintable.end(); ++btr){
                cout<<"\t pID:"<<btr->first<<" pAddr:"<<btr->second<< endl;
                btr->second->displayPinInfo();
            }
            cout << endl;
            //cout<<"\t Nid: " << nn->nid <<endl;  
    }
};
class graph{
    public:
    vector<int>node;
    map<int, nodes*>nodetable;
    nodes* nn;
    void addnode(nodes* ptr, int nid){
        node.push_back(nid);
        nodetable[nid]=ptr;
    }
    void displayGraph(){

        for(auto itr=nodetable.begin(); itr!=nodetable.end(); ++itr){
            cout<<"nID:"<<itr->first<<" nAddr:"<<itr->second<< endl;
            nn = itr->second;
            nn->displayNodeInfo();
        }
    }
    
};


void addEdge(pins* p1, pins* p2){
    p1->addPinEdge(p2->getpid());
    p2->addPinEdge(p1->getpid());
}

int main()
{
    graph* g1=new graph; 
    nodes* n1=new nodes;
    nodes* n2=new nodes;
    nodes* n3=new nodes;
    pins *p1=new pins;
    pins *p2=new pins;
    pins *p3=new pins;
    pins *p4=new pins;
    pins *p5=new pins;
    pins *p6=new pins;
    pins *p7=new pins;
    n1->setid(1);
    n2->setid(2);
    n3->setid(3);
    n1->setType(AND);
    n2->setType(NOT);
    n3->setType(OR);
    g1->addnode(n1, n1->getid());
    g1->addnode(n2, n2->getid());
    g1->addnode(n3, n3->getid());
    p1->setpid(101);
    p2->setpid(102);
    p3->setpid(103);
    p4->setpid(104);
    p5->setpid(105);
    p6->setpid(106);
    p7->setpid(107); 
    n1->addpin(p1, p1->getpid());
    n1->addpin(p2, p2->getpid());
    n1->addpin(p3, p3->getpid());
    n2->addpin(p4, p4->getpid());
    n2->addpin(p5, p5->getpid());
    n3->addpin(p6, p6->getpid());
    n3->addpin(p7, p7->getpid());
    
    //Add Edges
    addEdge(p3, p6);
    addEdge(p5, p7);
    g1->displayGraph();

    return 0;
}
