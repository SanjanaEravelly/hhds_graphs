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
    Pid pid : 32;
    Nid masterNid : 32;
    Port_id portId : 16;
    int64_t sedge : 48;       // Short-edges (48 bits 2-complement)

public:
    Pin() : pid(0), masterNid(0), portId(0) {clear_pin();}

    Pin(Pid pidValue, Nid mNidValue, Port_id portIdValue) {
        clear_pin();
        this->pid = pidValue;
        this->masterNid = mNidValue;
        this->portId = portIdValue;
    }
    void clear_pin(){
        bzero(this, sizeof(Pin));  // set everything to zero
        return;
    }
    Pid get_pid() const {
        return this->pid;
    }
    Nid get_nid() const {
        return this->masterNid;
    }
    Port_id get_portid() const {
        return this->portId;
    }
    bool add_edge(Pid self_id, Pid other_id) {
        assert(self_id != other_id);
        cout<<"Adding edge between pins "<<self_id <<" and "<<other_id <<endl;

        // Check if any of the 4th 12 bits is set
        if((sedge>>3*12 & 0xFFF) != 0) {
            cout<<"Maximum sedges reached. Need overflow handling" <<endl<<endl;
            return false;
        }

        // Typecast to avoid underflow
        int64_t diff = static_cast<int32_t>(other_id) - static_cast<int32_t>(self_id);

        //fits if diff is between -2048 to 2047
        bool fits = diff > -(1 << 11) && diff < ((1 << 11) - 1);  // 12 bits 2-complement

        if(!fits) {
            cout<<"Failed adding edge: !fits; diff="<<diff <<endl<<endl;
            return false;
        }

        // Try to add the edge to one of the 4 positions
        for (int i = 0; i < 4; ++i) {
            // Check if this edge position is available
            if ((sedge & (0xFFFLL << (i * 12))) == 0) { // 12 bits mask
                // Store the new edge in the next available space in sedge
                sedge |= (diff & 0xFFF) << (i * 12); // Store each edge in 12 bits
                cout <<"Added edge: i:"<< i <<" diff=" << diff << endl << endl;
                return true;
                }
        }
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
        return this->sedge != 0;
    }

    vector<int32_t> get_sedges() const {
        vector<int32_t> edges;
        for (int i = 0; i < 4; ++i) {
            // Extract each 12-bit edge
            int32_t edge = (sedge >> (i * 12)) & 0xFFF; // Get the 12 bits

            // Check for sign extension (12-bit signed to 32-bit signed)
            if (edge & 0x800) { // If the sign bit is set
                edge |= 0xFFFFF000; // Sign extend to 32 bits
            }

            if (edge != 0) { // Only add non-zero edges
                edges.push_back(this->pid+edge);
            }
        }
        return edges;
    }
};

class __attribute__((packed)) Node{
private:
    Nid nid : 32;
    Type type : 16;
    
public:
    // Default constructor
    Node() : nid(0) {clear_node();} // Initialize with default values

    Node(uint32_t nidValue) {
        clear_node();
        this->nid = nidValue;
    }

    void clear_node(){
        bzero(this, sizeof(Node));  // set everything to zero
        return;
    }
    
    void set_type(Type type){
        this->type = type;
    }

    Nid get_nid() const {
        return this->nid;
    }

    Type get_type() const {
        return this->type;
    }
};

// graph_class 
class __attribute__((packed)) Graph{
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
        Node newNode(id);
        nodeTable[id] = newNode;
        //cout << "Created new Node: "<< id << endl;
        return id;
    }
    Pid create_pin(Nid nid, Port_id portid){
        Pid id = generate_pinID(); // Generate new PinID
        assert(id);
        if(pinTable.size() <= id){
            pinTable.resize(id+1);     // Resize pinTable to fit new pin
        }
        Pin newPin(id, nid, portid);  // Allocate memory for new pin
        pinTable[id] = newPin;
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
            if(currPin->has_edges()) {   //Currently only sedges
                vector<int32_t> edges = currPin->get_sedges();
                cout<<"\t Short edge(s): ";
                for (const auto& edge : edges) {
                    cout << edge << " ";
                }
                cout<<endl;
            }
        }
    }
};

static_assert(sizeof(Graph) == 48, "Graph size must be 48 bytes");
static_assert(sizeof(Node) == 6, "Node size must be 6 bytes");
static_assert(sizeof(Pin) == 16, "Graph size must be 12 bytes");

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
    g1.add_edge(5, 7);
    //g1.add_edge(4, 100000); //Should not fit for both pins, diff not in range
    g1.add_edge(6, 5);
    g1.add_edge(2, 5);
    g1.add_edge(5, 3); //Should overflow for 5, because 5 already has 4 sedge
    g1.add_edge(5, 4); //Should overflow for 5, because 5 already has 4 sedge
    g1.add_edge(10, 24);

    g1.display_graph();
    cout<<"Pin size: "<<sizeof(Pin) <<endl;


    return 0;
}

