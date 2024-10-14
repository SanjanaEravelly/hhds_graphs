#include <iostream>
#include <vector>
#include <cassert>

#define NUM_NODES 10
#define NUM_TYPES 3
#define MAX_PINS_PER_NODE 10

typedef uint32_t Nid;
typedef uint32_t Pid;
typedef uint16_t Type;
typedef uint16_t Port_id;

using namespace std;

int gNid = 1;
int gPid = 1;
 

class pins{
private:
    Pid pid;
    Nid masterNid;
    Port_id portId;
    
public:
    Pid getPid(){
        return this->pid;
    }
    void setPid(Pid pid){
        this->pid = pid;
    }
    Nid getNid(){
        return this->masterNid;
    }
    void setNid(Nid nid){
        this->masterNid = nid;
    }
    Port_id getPortid(){
        return this->portId;
    }
    void setPortid(Port_id portid){
        this->portId = portid;
    }
};

class nodes{
private:
    Nid nid;
    Type type;
    
public:

    Nid getNid(){
        return this->nid;
    }
    void setNid(Nid id){
        this->nid = id;
    }
    
    void setType(Type ty){
        this->type = ty;
    }
    Type getType(){
        return this->type;
    }
};

// graph_class 
class graph{
    public:
    vector<nodes*> nodeTable;      // array of nodes
    vector<pins*> pinTable;        // array of Pins

    graph() {clear_graph();}
    void clear_graph(){
        // TODO: Clear graph?
        return;
    }
    
    int generate_nodeID(){
        // We can create random Nids.
        // But to keep it simple, I am using a global variable and incrementing everytime we create node.
        //TODO: Add logic to avoid deleted Nids.
        return gNid++;
    }
    int generate_pinID(){
        // Same as generate_nodeID()
        //TODO: Add logic to avoid deleted Nids.
        return gPid++;
    }
    
    Nid create_node(){
        Nid id = generate_nodeID(); // Generate new NodeID
        assert(id);
        if(nodeTable.size() <= id){
            nodeTable.resize(id+1);     // Resize nodeTable to fit new node
        }
        nodes* newNode = new nodes;  // Allocate memory for new node
        nodeTable[id] = newNode;
        newNode->setNid(id);  // Set Nid of new node
        //cout << "Created new Node: "<< id << endl;
        return id;
    }
    Pid create_pin(Nid nid, Port_id portid){
        Pid id = generate_pinID(); // Generate new PinID
        assert(id);
        if(pinTable.size() <= id){
            pinTable.resize(id+1);     // Resize pinTable to fit new pin
        }
        pins* newPin = new pins;  // Allocate memory for new pin
        pinTable[id] = newPin;
        newPin->setPid(id);  // Set Pid of new pin
        newPin->setNid(nid);  // Set Pid of new pin
        newPin->setPortid(portid);  // Set PortId of new pin
        //cout << "Created new Pin "<<id <<" for node "<<nid <<endl;
        return id;
    }
    
    void set_type(Nid nid, Type type){
        ref_node(nid)->setType(type);
    }
    
    nodes* ref_node(Nid id){
        assert(id);
        return (nodes* )nodeTable[id];
    }
    pins* ref_pin(Pid id){
        assert(id);
        return (pins* )pinTable[id];
    }

    //Visualize entire graph. This is just for development purpose
    void displayGraph(){
        for (int i = 1; i < pinTable.size(); ++i) {
            pins* currPin = ref_pin(i);
            cout<<"PinID: "<<currPin->getPid() <<endl;
            cout<<"\t MasterNodeID: "<<currPin->getNid();
            cout<<"; NodeType: "<<ref_node(currPin->getNid())->getType() <<endl;
            cout<<"\t PortID: "<<currPin->getPortid() <<endl;
            cout<<endl;
        }
    }
};

int main()
{
    std::vector<Nid> node_id;
    std::vector<Pid> pin_id;
    
    std::srand(std::time(0));
    
    graph g1;
    
    for(int i=0; i<NUM_NODES; i++){
        //Create Nodes
        Nid nid = g1.create_node();
        node_id.push_back(nid); //Dosnt create extra copy like push_back does
        
        //Add node type
        g1.set_type(nid, nid % NUM_TYPES);
        
        //Add pins to node as required
        int rpins = std::rand() % MAX_PINS_PER_NODE + 1;
        for(int i=0; i<rpins; i++){
            Pid pid = g1.create_pin(nid, 1);
            pin_id.push_back(pid);
        }
    }

    g1.displayGraph();
    
/*
    Questions:
    1. Should we not have any pin details in class node?
    2. For adding edges - Should we consider driver_id as source and sink_id as destination?
*/

    return 0;
}

