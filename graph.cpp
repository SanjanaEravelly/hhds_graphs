#include <iostream>
#include <vector>
#include <cassert>
//#include "iassert.hpp"

#define NUM_NODES 10
#define NUM_TYPES 3
#define MAX_PINS_PER_NODE 10

typedef uint32_t Nid;
typedef uint32_t Pid;
typedef uint16_t Type;
typedef uint16_t Port_id;

//using namespace std;

int gNid = 1;
int gPid = 1;

class __attribute__((packed)) Pin{
private:
    Nid master_nid : 32;
    Port_id port_id : 16;
    int64_t sedge : 48;       // Short-edges (48 bits 2-complement)

public:
    Pin() : master_nid(0), port_id(0) {clear_pin();}

    Pin(Nid master_nid_value, Port_id port_id_value) {
        clear_pin();
        master_nid = master_nid_value;
        port_id = port_id_value;
    }
    void clear_pin(){
        bzero(this, sizeof(Pin));  // set everything to zero
        return;
    }
    Nid get_master_nid() const {
        return master_nid;
    }
    Port_id get_port_id() const {
        return port_id;
    }
    bool add_edge(Pid self_id, Pid other_id) {
        assert(self_id != other_id);
        std::cout<<"Adding edge between pins "<<self_id <<" and "<<other_id <<std::endl;

        // Check if any of the 4th 12 bits is set
        if((sedge>>3*12 & 0xFFF) != 0) {
            std::cout<<"Maximum sedges reached. Need overflow handling" <<std::endl<<std::endl;
            return false;
        }

        // Typecast to avoid underflow
        int64_t diff = static_cast<int32_t>(other_id) - static_cast<int32_t>(self_id);

        //fits if diff is between -2048 to 2047
        bool fits = diff > -(1 << 11) && diff < ((1 << 11) - 1);  // 12 bits 2-complement

        if(!fits) {
            std::cout<<"Failed adding edge: !fits; diff="<<diff <<std::endl<<std::endl;
            return false;
        }

        // Try to add the edge to one of the 4 positions
        for (int i = 0; i < 4; ++i) {
            // Check if this edge position is available
            if ((sedge & (0xFFFLL << (i * 12))) == 0) { // 12 bits mask
                // Store the new edge in the next available space in sedge
                sedge |= (diff & 0xFFF) << (i * 12); // Store each edge in 12 bits
                std::cout <<"Added edge: i:"<< i <<" diff=" << diff << std::endl << std::endl;
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
        return sedge != 0;
    }

    std::array<int32_t, 4> get_sedges(Pid pid) const {
        std::array<int32_t, 4> edges = {0, 0, 0, 0};
        int edge_count = 0;
        for (int i = 0; i < 4; ++i) {
            // Extract each 12-bit edge
            int32_t edge = (sedge >> (i * 12)) & 0xFFF; // Get the 12 bits

            // Check for sign extension (12-bit signed to 32-bit signed)
            if (edge & 0x800) { // If the sign bit is set
                edge |= 0xFFFFF000; // Sign extend to 32 bits
            }

            if (edge != 0 && edge_count < 4) {
                edges[edge_count++] = pid + edge;
            }
        }
        return edges;
    }
};

class __attribute__((packed)) Node{
private:
    Nid nid : 32;
    Type type : 16;
    //Pid next_pin_ptr;
    
public:
    // Default constructor
    Node() : nid(0) {clear_node();} // Initialize with default values

    Node(uint32_t nid_value) {
        clear_node();
        nid = nid_value;
    }

    void clear_node(){
        bzero(this, sizeof(Node));  // set everything to zero
        return;
    }
    
    void set_type(Type type){
        type = type;
    }

    Nid get_nid() const {
        return nid;
    }

    Type get_type() const {
        return type;
    }
};

// graph_class 
class __attribute__((packed)) Graph{
    public:
    std::vector<Node> node_table;      // array of nodes
    std::vector<Pin> pin_table;        // array of Pins

    Graph() {clear_graph();}
    void clear_graph(){
        bzero(this, sizeof(Graph));     // set everything to zero
        node_table.emplace_back(0);     // To avoid assertion for size=0
        pin_table.emplace_back(0,0);  // To avoid assertion for size=0
        return;
    }
    
    Nid create_node(){
        Nid id = node_table.size(); // Generate new NodeID
        assert(id);
        node_table.emplace_back(id);
        return id;
    }
    Pid create_pin(Nid nid, Port_id port_id){
        Pid id = pin_table.size(); // Generate new PinID
        assert(id);
        pin_table.emplace_back(nid, port_id);
        return id;
    }

    Node* ref_node(Nid id) const {
        assert(id);
        return (Node* )&node_table[id];
    }
    Pin* ref_pin(Pid id) const {
        assert(id);
        return (Pin* )&pin_table[id];
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
        std::cout<<"add_edge_int failed: " <<self_id <<" " <<other_id <<std::endl;
    }

    //Visualize entire graph. This is just for development purpose
    void display_graph() const {
        for (Pid pid = 1; pid < pin_table.size(); ++pid) {
            Pin* currPin = ref_pin(pid);
            std::cout<<"Pin ID: "<< pid <<std::endl;
            std::cout<<"\t Master Node ID: "<<currPin->get_master_nid();
            std::cout<<"; Node Type: "<<ref_node(currPin->get_master_nid())->get_type() <<std::endl;
            std::cout<<"\t Port ID: "<<currPin->get_port_id() <<std::endl;
            if(currPin->has_edges()) {   //Currently only sedges
                std::array<int32_t, 4> edges = currPin->get_sedges(pid);
                std::cout<<"\t Short edge(s): ";
                for (const auto& edge : edges) {
                    std::cout << edge << " ";
                }
                std::cout<<std::endl;
            }
        }
    }
};

static_assert(sizeof(Graph) == 48, "Graph size must be 48 bytes");
static_assert(sizeof(Node) == 6, "Node size must be 6 bytes");
static_assert(sizeof(Pin) == 12, "Graph size must be 12 bytes");

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
        g1.ref_node(nid)->set_type(nid % NUM_TYPES);
        
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
    std::cout<<"Pin size: "<<sizeof(Pin) <<std::endl;


    return 0;
}

