
#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <cmath>
#include <algorithm>

struct Vector2i {
    int x, y;
    Vector2i(int a=0,int b=0):x(a),y(b){}
    Vector2i operator+(const Vector2i &o) const { return Vector2i(x+o.x, y+o.y); }
    bool operator==(const Vector2i &o) const { return x==o.x && y==o.y; }
};

// Hash for Vector2i for unordered_map/set
struct Vector2iHash {
    size_t operator()(const Vector2i &v) const {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

struct Entity {
    Vector2i position;
    float move_timer = 1.0f;
    int step_index = 0; // deterministic movement

    Entity(Vector2i pos):position(pos){}

    void reset_timer() { move_timer = 1.0f; }

    bool simulate(float delta, Vector2i &out_new_pos) {
        move_timer -= delta;
        if(move_timer > 0.0f) { out_new_pos = position; return false; }

        static const Vector2i dirs[4] = { {1,0}, {0,1}, {-1,0}, {0,-1} };
        out_new_pos = position + dirs[step_index];
        step_index = (step_index + 1) % 4;
        reset_timer();
        return true;
    }
};

struct World; // forward

struct Chunk {
    int width, height;
    Vector2i coord;
    World* world;
    std::vector<std::shared_ptr<Entity>> entities;

    Chunk(int w,int h,Vector2i c, World* wld):width(w),height(h),coord(c),world(wld){}

    void simulate(float delta, bool full_simulation);

    void transfer_entities(std::vector<std::shared_ptr<Entity>> &list, Vector2i dir);
};

struct World {
    int chunk_size = 4;
    int world_chunks_x = 4;
    int world_chunks_y = 4;

    std::unordered_map<Vector2i,std::shared_ptr<Chunk>,Vector2iHash> chunks;

    struct Transfer {
        Vector2i target_chunk;
        std::shared_ptr<Entity> entity;
    };
    std::vector<Transfer> transfer_queue;

    std::shared_ptr<Chunk> get_chunk(const Vector2i &c) {
        auto it = chunks.find(c);
        return it!=chunks.end()?it->second:nullptr;
    }

    std::shared_ptr<Chunk> load_chunk(const Vector2i &c) {
        auto existing = get_chunk(c);
        if(existing) return existing;
        auto ch = std::make_shared<Chunk>(chunk_size, chunk_size, c, this);
        chunks[c] = ch;
        return ch;
    }

    bool is_valid_chunk(const Vector2i &c) { 
        return c.x>=0 && c.x<world_chunks_x && c.y>=0 && c.y<world_chunks_y;
    }

    void queue_entity_transfer(const std::vector<std::shared_ptr<Entity>> &list,const Vector2i &target) {
        for(auto &e: list) transfer_queue.push_back({target,e});
    }

    void apply_transfers() {
        for(auto &t: transfer_queue) {
            auto ch = get_chunk(t.target_chunk);
            if(!ch) ch = load_chunk(t.target_chunk);
            ch->entities.push_back(t.entity);
        }
        transfer_queue.clear();
    }

    void update(float delta) {
        // full sim for all loaded chunks
        for(auto &kv: chunks) kv.second->simulate(delta,true);
        apply_transfers();

for(auto& kv : chunks){
    std::cout << "Chunk("<<kv.first.x<<","<<kv.first.y<<") has "<<kv.second->entities.size()<<" entities\n";
}

    }
};

void Chunk::simulate(float delta, bool full_simulation) {
    std::vector<std::shared_ptr<Entity>> to_transfer[4]; // N,E,S,W
    std::vector<int> to_clear;

    for(int i=0;i<(int)entities.size();i++){
        if(!entities[i]) continue;
        Vector2i new_pos;
        bool moved = entities[i]->simulate(delta, full_simulation ? new_pos : entities[i]->position);
        if(!moved) continue;

        if(new_pos.x < 0){ to_transfer[3].push_back(entities[i]); to_clear.push_back(i);}
        else if(new_pos.x >= width){ to_transfer[1].push_back(entities[i]); to_clear.push_back(i);}
        else if(new_pos.y < 0){ to_transfer[0].push_back(entities[i]); to_clear.push_back(i);}
        else if(new_pos.y >= height){ to_transfer[2].push_back(entities[i]); to_clear.push_back(i);}
        else { entities[i]->position = new_pos; }
    }

    if(!to_transfer[0].empty()) world->queue_entity_transfer(to_transfer[0],coord+Vector2i(0,-1));
    if(!to_transfer[1].empty()) world->queue_entity_transfer(to_transfer[1],coord+Vector2i(1,0));
    if(!to_transfer[2].empty()) world->queue_entity_transfer(to_transfer[2],coord+Vector2i(0,1));
    if(!to_transfer[3].empty()) world->queue_entity_transfer(to_transfer[3],coord+Vector2i(-1,0));

    std::cout << "Chunk " << coord.x << "," << coord.y << " clearing " << to_clear.size() << " entities\n";
    for(int idx: to_clear) if(idx>=0 && idx<(int)entities.size()) entities[idx] = nullptr;
}

int main(){
    World world;
    int chunks = 2;

    world.world_chunks_x = chunks;
    world.world_chunks_y = chunks;
    world.chunk_size = chunks * chunks;

    // Create chunks and entities
    for(int y=0;y<chunks;y++){
        for(int x=0;x<chunks;x++){
            auto ch = world.load_chunk({x,y});
            for(int i=0;i<3;i++){
                ch->entities.push_back(std::make_shared<Entity>(Vector2i(i * x,i * y)));
            }
        }
    }

    for(int frame=0;frame<100;frame++){
        world.update(1.0f);

            std::cout << "Frame " << frame << "\n";
            for(int y=0;y<chunks;y++){
                for(int x=0;x<chunks;x++){
                    auto ch = world.get_chunk({x,y});
                    std::cout << "Chunk("<<x<<","<<y<<"): ";
                    for(auto &e: ch->entities){
                        if(e) std::cout << "("<<e->position.x<<","<<e->position.y<<") ";
                    }
                    std::cout << "\n";
                }
            }
            std::cout << "--------------------\n";
    }
}
