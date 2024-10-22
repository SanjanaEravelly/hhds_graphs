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
 

class __attribute__((packed)) Pin{
private:
    Pid pid;
    Nid masterNid;
    Port_id portId;
    int16_t sedge_0 : 11;       // Short-edge (11 bits 2-complement)

public:
    Pin() {clear_pin();}
    void clear_pin(){
        bzero(this, sizeof(Pin));  // set everything to zero
        return;
    }
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
    bool add_edge(Pid self_id, Pid other_id) {
        assert(self_id != other_id);
        cout<<"Adding edge between pins "<<self_id <<" and "<<other_id <<endl;
        if(sedge_0 == 0) {
            // Typecast to avoid underflow
            int64_t diff = static_cast<int32_t>(other_id) - static_cast<int32_t>(self_id);
            //fits if diff is between -1024 to 1023
            bool fits = diff > -(1 << 10) && diff < ((1 << 10) - 1);  // 11 bits 2-complement
            if(fits) {
                sedge_0 = static_cast<int16_t>(diff);
                cout<<"Added edge: diff="<<diff <<" sedge_0="<<sedge_0 <<endl<<endl;
                return true;
            }
            cout<<"Failed adding edge: !fits; diff="<<diff <<endl<<endl;
            return false;
        }
        cout<<"Alread edge 0 exist. Need overflow handling. existing edge="<<sedge_0 <<endl<<endl;
        return false;
#if 0
        //Overflow is not handled for now
        if(overflow_link | set_link) {
            return false;
        }
        ledge_or_overflow_or_set = other_id;
        return true;
#endif
    }
    bool has_edges() const {
        return this->sedge_0 != 0;
    }
    uint16_t get_edge0() const {
        return (this->pid+this->sedge_0);
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
    
    void add_edge(uint32_t driver_id, uint32_t sink_id) const {
        add_edge_int(driver_id, sink_id);
        add_edge_int(sink_id, driver_id);
    }
    void add_edge_int(uint32_t self_id, uint32_t other_id) const {
        // For now considering only Pins have edge(s)
        bool ok = ref_pin(self_id)->add_edge(self_id, other_id);
        if(ok){
            return;
        }
        cout<<"add_edge_int failed: " <<self_id <<" " <<other_id <<endl;
    }

    //Visualize entire graph. This is just for development purpose
    void display_graph() const {
        for (int i = 1; i < pinTable.size(); ++i) {
            Pin* currPin = ref_pin(i);
            cout<<"PinID: "<<currPin->get_pid() <<endl;
            cout<<"\t MasterNodeID: "<<currPin->get_nid();
            cout<<"; NodeType: "<<ref_node(currPin->get_nid())->get_type() <<endl;
            cout<<"\t PortID: "<<currPin->get_portid() <<endl;
            if(currPin->has_edges())    //Currently only edge0
                cout<<"\t edge 0: "<<currPin->get_edge0() <<endl;
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

    g1.add_edge(2, 3);
    g1.add_edge(10, 20);
    g1.add_edge(5, 1);
    g1.add_edge(5, 7); //Should overflow for 5, because 5 already has a edge
    //g1.add_edge(4, 100000); //Should not fit for both pins, diff not in range

    g1.display_graph();
    
    return 0;
}

