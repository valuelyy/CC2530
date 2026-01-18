#include <ioCC2530.h>
#include "traffic_light.h"

// 全局变量声明（带完整类型）
char zt = 'r';
char kzfold = 'f';
char kzfnew = 'r';

// 函数原型声明
void initialize_system(void);
void process_commands(void);

void main(void) {
    initialize_system();
    
    while (1) {
        process_commands();
        // 第20行应该在循环体内
        zt = get_current_mode();  // 示例代码
    }
}
