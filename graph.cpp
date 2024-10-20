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
 

class Pin{
private:
    Pid pid;
    Nid masterNid;
    Port_id portId;
    
public:
    Pid get_pid() const {
        return this->pid;
    }
    void set_pid(Pid pid){
        this->pid = pid;
    }
    Nid get_nid() const {
        return this->masterNid;
    }
    void set_nid(Nid nid){
        this->masterNid = nid;
    }
    Port_id get_portid() const {
        return this->portId;
    }
    void set_portid(Port_id portid){
        this->portId = portid;
    }
};

class Node{
private:
    Nid nid;
    Type type;
    
public:

    Nid get_nid() const {
        return this->nid;
    }
    void set_nid(Nid id){
        this->nid = id;
    }
    
    void set_type(Type ty){
        this->type = ty;
    }
    Type get_type() const {
        return this->type;
    }
};

// graph_class 
class Graph{
    public:
    vector<Node> nodeTable;      // array of nodes
    vector<Pin> pinTable;        // array of Pins

    void graph() {clear_graph();}
    void clear_graph(){
        // TODO: Clear graph?
        return;
    }
    
    int generate_nodeID() const {
        // We can create random Nids.
        // But to keep it simple, I am using a global variable and incrementing everytime we create node.
        //TODO: Add logic to avoid deleted Nids.
        return gNid++;
    }
    int generate_pinID() const {
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
        Node newNode;
        nodeTable[id] = newNode;
        nodeTable[id].set_nid(id);  // Set Nid of new node
        //cout << "Created new Node: "<< id << endl;
        return id;
    }
    Pid create_pin(Nid nid, Port_id portid){
        Pid id = generate_pinID(); // Generate new PinID
        assert(id);
        if(pinTable.size() <= id){
            pinTable.resize(id+1);     // Resize pinTable to fit new pin
        }
        Pin newPin;  // Allocate memory for new pin
        pinTable[id] = newPin;
        pinTable[id].set_pid(id);  // Set Pid of new pin
        pinTable[id].set_nid(nid);  // Set Pid of new pin
        pinTable[id].set_portid(portid);  // Set PortId of new pin
        //cout << "Created new Pin "<<id <<" for node "<<nid <<" with portID: "<<portid <<endl;
        return id;
    }
    
    void set_type(Nid nid, Type type) const {
        ref_node(nid)->set_type(type);
    }
    
    Node* ref_node(Nid id) const {
        assert(id);
        return (Node* )&nodeTable[id];
    }
    Pin* ref_pin(Pid id) const {
        assert(id);
        return (Pin* )&pinTable[id];
    }

    //Visualize entire graph. This is just for development purpose
    void display_graph() const {
        for (int i = 1; i < pinTable.size(); ++i) {
            Pin* currPin = ref_pin(i);
            cout<<"PinID: "<<currPin->get_pid() <<endl;
            cout<<"\t MasterNodeID: "<<currPin->get_nid();
            cout<<"; NodeType: "<<ref_node(currPin->get_nid())->get_type() <<endl;
            cout<<"\t PortID: "<<currPin->get_portid() <<endl;
            cout<<endl;
        }
    }
};

int main()
{
    std::vector<Nid> node_id;
    std::vector<Pid> pin_id;
    
    std::srand(std::time(0));
    
    Graph g1;
    
    for(int i=0; i<NUM_NODES; i++){
        //Create Nodes
        Nid nid = g1.create_node();
        node_id.push_back(nid);
        
        //Add node type
        g1.set_type(nid, nid % NUM_TYPES);
        
        //Add pins to node as required
        int rpins = std::rand() % MAX_PINS_PER_NODE + 1;
        for(int i=0; i<rpins; i++){
            Pid pid = g1.create_pin(nid, 1);
            pin_id.push_back(pid);
        }
    }

    g1.display_graph();
    
    return 0;
}

