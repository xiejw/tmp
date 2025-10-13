#define NODISCARD __attribute__((warn_unused_result))
struct t {int x;} NODISCARD;
NODISCARD int hello( ) { return 2; }
struct t hello2( ) { struct t x = {0}; return x; }
int main( ) { (void)hello( ); (void)hello2(); return 0; }
