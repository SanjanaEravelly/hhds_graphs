#include <iostream>
#include <vector>
#include <cassert>
#include "hash_set3.hpp"

constexpr int NUM_NODES = 10;
constexpr int NUM_TYPES = 3;
constexpr int MAX_PINS_PER_NODE = 10;

using Nid = uint32_t;
using Pid = uint32_t;
using Type = uint16_t;
using Port_id = uint16_t;

class __attribute__((packed)) Pin{
private:
    Nid master_nid : 32;
    Port_id port_id : 16;
    int64_t sedge : 48;       // Short-edges (48 bits 2-complement)
    Pid next_pin_id : 32;     // Points to next pin of the same master_node
    emhash7::HashSet<Pid> edges;

public:
    Pin() : master_nid(0), port_id(0), sedge(0), next_pin_id(0) {}
    Pin(Nid master_nid_value, Port_id port_id_value)
        : master_nid(master_nid_value), port_id(port_id_value), sedge(0), next_pin_id(0) {}

    [[nodiscard]] auto get_master_nid() const -> Nid {
        return master_nid;
    }
    [[nodiscard]] auto get_port_id() const -> Port_id {
        return port_id;
    }
    bool overflow_handling(Pid other_id){
        edges.insert(other_id);
        return true;
    }
    bool add_edge(Pid self_id, Pid other_id) {
        assert(self_id != other_id);
        std::cout<<"Adding edge between pins "<<self_id <<" and "<<other_id <<std::endl;

        //Assuming all edges goes to overflow:
        overflow_handling(other_id);

        return true;
#if 0
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
#endif
#if 0
        //Overflow is not handled for now
        if(overflow_link | set_link) {
            return false;
        }
        ledge_or_overflow_or_set = other_id;
        return true;
#endif
    }
    [[nodiscard]] auto has_edges() const -> bool {
        return sedge != 0;
    }

    [[nodiscard]] std::array<int32_t, 4> get_sedges(Pid pid) const {
        std::array<int32_t, 4> edges = {0, 0, 0, 0};
        int edge_count = 0;
        for (int i = 0; i < 4; ++i) {
            // Extract each 12-bit edge
            int32_t edge = (sedge >> (i * 12)) & 0xFFF; // Get the 12 bits

            // Check for sign extension (12-bit signed to 32-bit signed)
            if (edge & 0x800) { // If the sign bit is set
                edge |= 0xFFFFF000; // Sign extend to 32 bits
            }

            if (edge != 0) {
                edges[edge_count++] = pid + edge;
            }
        }
        return edges;
    }

    void print_edges() {
        std::cout << "\t Edges:";
         for (const auto &edge : edges) {
             std::cout << " " << edge;
         }
         std::cout << std::endl;
    }

    [[nodiscard]] auto get_next_pin_id() const -> Pid {
        return next_pin_id;
    }
    void set_next_pin_id(Pid id) {
        next_pin_id = id;
    }
};

class __attribute__((packed)) Node{
private:
    Nid nid : 32;
    Type type : 16;
    Pid next_pin_id;       // Points to the first pin of the node
    
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

    [[nodiscard]] auto get_nid() const -> Nid {
        return nid;
    }

    [[nodiscard]] auto get_type() const -> Type {
        return type;
    }
    [[nodiscard]] auto get_next_pin_id() const -> Pid {
        return next_pin_id;
    }
    void set_next_pin_id(Pid id) {
        next_pin_id = id;
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
    
    [[nodiscard]] auto create_node() -> Nid {
        Nid id = node_table.size(); // Generate new NodeID
        assert(id);
        node_table.emplace_back(id);
        return id;
    }
    [[nodiscard]] auto create_pin(Nid nid, Port_id port_id) -> Pid{
        Pid id = pin_table.size(); // Generate new PinID
        assert(id);
        pin_table.emplace_back(nid, port_id);
        //ref_node(nid)->set_next_pin_ptr(ref_pin(id));
        set_next_pin(nid, id);
        return id;
    }

    [[nodiscard]] Node* ref_node(Nid id) const {
        assert(id);
        return (Node* )&node_table[id];
    }
    [[nodiscard]] Pin* ref_pin(Pid id) const {
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
    
    void set_next_pin(Nid nid, Pid next_pin){
        if(ref_node(nid)->get_next_pin_id() == 0) {     // If Node does not have any pin
            ref_node(nid)->set_next_pin_id(next_pin);   // Set first pin's pointer in node
            return;
        }
        Pid next_pid = ref_node(nid)->get_next_pin_id();
        // Move across all the existing pins and find the end of the list
        while(pin_table[next_pid].get_next_pin_id()) {
            next_pid = pin_table[next_pid].get_next_pin_id();
        }
        pin_table[next_pid].set_next_pin_id(next_pin);  //Set the next pin ptr to each pin
        return;
    }

    //Visualize entire graph. This is just for development purpose
    void display_graph() const {
        for (Pid pid = 1; pid < pin_table.size(); ++pid) {
            Pin* currPin = ref_pin(pid);
            std::cout<<"Pin ID: "<< pid <<std::endl;
            std::cout<<"\t Master Node ID: "<<currPin->get_master_nid();
            std::cout<<"; Node Type: "<<ref_node(currPin->get_master_nid())->get_type() <<std::endl;
            std::cout<<"\t Port ID: "<<currPin->get_port_id() <<std::endl;
            /*
            if(currPin->has_edges()) {   //Currently only sedges
                std::array<int32_t, 4> edges = currPin->get_sedges(pid);
                std::cout<<"\t Short edge(s): ";
                for (const auto& edge : edges) {
                    std::cout << edge << " ";
                }
                std::cout<<std::endl;
            }
            std::cout<<"\t Next Pid: "<<currPin->get_next_pin_id() <<std::endl;
            */
            currPin->print_edges();
        }
    }
    void display_next_pin_of_node(){
        for (Nid nid = 1; nid < node_table.size(); ++nid) {
            Node* currNode = ref_node(nid);
            std::cout<<"Node ID: "<< nid <<std::endl;
            std::cout<<"\t First pin of node: "<<currNode->get_next_pin_id() <<std::endl;
        }
    }
};

static_assert(sizeof(Graph) == 48, "Graph size must be 48 bytes");
static_assert(sizeof(Node) == 10, "Node size must be 6 bytes");
static_assert(sizeof(Pin) == 64, "Pin size must be 64 bytes");

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
    g1.display_next_pin_of_node();

    return 0;
}

