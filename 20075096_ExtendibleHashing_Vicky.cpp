#include<bits/stdc++.h>
using namespace std;

#define ll long long

class Bucket{
    public:
        ll depth, size;
        vector<ll> records;
        Bucket(ll depth, ll size);
        ll insertIntoBckt(ll key);
        ll removeFromBckt(ll key);
        ll searchInBckt(ll key);
        ll bcktIsFull(void);
        void clrBckt(void);
        void displayBckt(void);
};

Bucket::Bucket(ll depth, ll size){
    this->depth = depth;
    this->size = size;
}

ll Bucket::insertIntoBckt(ll key){
    records.push_back(key);
}

ll Bucket::removeFromBckt(ll key){
    auto it = find(records.begin(), records.end(), key);
    records.erase(it);
}

ll Bucket::searchInBckt(ll key){
    auto it = find(records.begin(), records.end(), key);
    if(it != records.end()) return 1;
    else return 0;
}

ll Bucket::bcktIsFull(){
    if(records.size() == size) return 1;
    else return 0;
}

void Bucket::clrBckt(){
    records.clear();
}

void Bucket::displayBckt(){
    cout << "Local Depth: " << depth << " Number of Keys: " << records.size() <<  endl;
    for(int i=0;i<records.size();i++){
        cout << records[i] << " ";
    }
    cout << endl;
}



class Directory{
    public:
        ll gd, bckt_sz;
        vector<Bucket*> bckts;
        Directory(ll depth, ll bckt_sz);
        ll hash_func(ll x);
        ll lastKBits(ll x, ll k);
        ll directoryNum(ll key);
        ll matchingBckts(Bucket *bckt, ll dir_num);
        void display(void);
        void grow(void);
        void split(Bucket *bckt, ll dir_num);
        ll searchKey(ll key);
        ll insertKey(ll key);
        void shrink(void);
        void merge(ll dir_num);
        ll removeKey(ll key);
        virtual ~Directory();
};

Directory::Directory(ll depth, ll bckt_sz){
    this->gd = depth;
    this->bckt_sz = bckt_sz;
    for(ll i=0; i < 1<<depth; i++){
        bckts.push_back(new Bucket(depth, bckt_sz));
    }
}

ll Directory::hash_func(ll x){
    ll hash = x;
    return hash;
}

ll Directory::lastKBits(ll x, ll k){
    ll y = ((1<<k)-1)&x;
    return y;
}

ll Directory::directoryNum(ll key){
    ll hash = hash_func(key);
    ll dir_num = lastKBits(hash, gd);
    return dir_num;
}

ll Directory::matchingBckts(Bucket *bckt, ll dir_num){
    ll x = bckt->depth;
    ll bckt_num = lastKBits(dir_num, x);
    ll matching_bckts = bckt_num^(1<<(x-1));
    return matching_bckts;
}

void Directory::display(){
    cout << "Global Depth: " << gd << endl;
    for(ll i=0; i<(ll)bckts.size(); i++){
        Bucket *bckt = bckts[i];
        if(i>>(bckt->depth) == 0){
            cout << i << ": ";
            bckt->displayBckt();
            cout << endl;
        }
        else{
            cout << i << ": Points to ";
            cout << lastKBits(i, bckt->depth);
            cout << endl;
        }
    }
}

void Directory::grow(){
    ll blocks = (1<<gd);
    bckts.resize(blocks*2);
    for(int i=0; i<blocks; i++){
        bckts[i^(1<<gd)] = bckts[i];
    }
    gd++;
}

void Directory::split(Bucket *bckt, ll dir_num){
    if(bckt->depth == gd) {
        grow();
    }

    ll ld = bckt->depth;
    ll bckt_num = lastKBits(dir_num, ld);
    ll bckt1 = lastKBits(dir_num, ld+1);
    ll bckt2 = ((1<<ld)^bckt1);

    bckt->depth++;
    bckts[bckt2] = new Bucket(bckt->depth,bckt_sz);
    
    for(ll i=0; i<(1<<(gd-ld)); i++){
        ll j = (i<<ld) + bckt_num;
    
        if(bckts[j] == bckt){
            if(lastKBits(j, ld+1) == bckt1) bckts[j] = bckts[bckt1];
            else bckts[j] = bckts[bckt2];
        }
    }
    vector<ll> tmp;
    for(ll key: bckt->records){
        tmp.push_back(key);
    }
    
    bckt->clrBckt();
    for(ll key: tmp){
        Bucket *bkt = bckts[directoryNum(key)];
        bkt->insertIntoBckt(key);
    }
}

ll Directory::searchKey(ll key){
    ll dir_num = directoryNum(key);
    Bucket *bckt = bckts[dir_num];
    return bckt->searchInBckt(key);
}

ll Directory::insertKey(ll key){
    if(searchKey(key)) return 0;
    int dir_num = directoryNum(key);
    Bucket *bckt = bckts[dir_num];
    if (!bckt->bcktIsFull()) {
        bckt->insertIntoBckt(key);
    }
    else {
        split(bckt, dir_num);
        insertKey(key);
    }
    return 1;
}

void Directory::shrink(){
    ll cnt = 0;
    for(ll i=0; i<(1<<gd); i++){
        if(bckts[i]->depth < gd){
        cnt++;
        }
    }

    if(cnt == (1<<gd)){
        gd--;
        bckts.resize(1<<gd);
    }
}

void Directory::merge(ll dir_num){
    Bucket *bckt = bckts[dir_num];
    ll ld = bckt->depth;
    if(ld == 0) return;
    int bckt2 = matchingBckts(bckt, dir_num);
    Bucket *b2 = bckts[bckt2];

    if((ll)(bckt->records).size() + (ll)(b2->records).size() <= bckt_sz && b2->depth == ld){
        for(auto key: b2->records){
            (bckt->records).push_back(key);
        }
    
        for(ll i=0; i<(1<<(gd-ld)); i++){
            ll j = ((i<<ld) + bckt2);
            if(bckts[j] == b2) bckts[j] = bckt;
        }

        bckt->depth--;
        delete b2;
        shrink();

        merge(lastKBits(dir_num, bckt->depth));
    }
}

ll Directory::removeKey(ll key){
    ll dir_num = directoryNum(key);
    Bucket *bckt = bckts[dir_num];
    if(!bckt->searchInBckt(key)) return 0;
    bckt->removeFromBckt(key);
    
    merge(dir_num);
    return 1;
}

Directory::~Directory(){
    for(ll i=0; i<bckts.size(); i++){
        if(bckts[i] == NULL) continue;
        (bckts[i])->clrBckt();
        delete((bckts[i]));
    }
}

int main(){
    ll max_bckt_sz, gd;
    ll key;
    ll operation;

    cout << "Enter the initial global depth: ";
    cin >> gd;

    cout << "Enter the maximum bucket size: ";
    cin >> max_bckt_sz;

    Directory d(gd, max_bckt_sz);

    cout << "Enter 1 for insertion\nEnter 2 for deletion\nEnter 3 for status\nEnter 4 to exit\n";
    cin >> operation;
    while(operation != 4){
        switch(operation){
            case 1:{
                cout << "Enter key to be inserted: ";
                cin >> key;
                d.insertKey(key);
                break;
            }
            case 2:{
                cout << "Enter key to be deleted: ";
                cin >> key;
                d.removeKey(key);
                break;
            }
            case 3:{
                d.display();
                break;
            }
        }
        cout << "Enter 1 for insertion\nEnter 2 for deletion\nEnter 3 for status\nEnter 4 to exit\n";
        cin >> operation;
    }
    return 0;
}