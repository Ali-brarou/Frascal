; ModuleID = 'main_module'
source_filename = "main_module"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %w = alloca i1, align 1
  store i32 0, ptr %y, align 4
  store i32 3, ptr %x, align 4
  %loaded_var = load i32, ptr %y, align 4
  %fsubtmp = sub i32 8, %loaded_var
  store i32 %fsubtmp, ptr %x, align 4
  br label %if_block

if_block:                                         ; preds = %entry
  %loaded_var1 = load i32, ptr %x, align 4
  %eqcmptmp = icmp eq i32 %loaded_var1, 2
  br i1 %eqcmptmp, label %then_block, label %elif_block

then_block:                                       ; preds = %if_block
  store i32 3, ptr %y, align 4
  br label %merge_block

elif_block:                                       ; preds = %if_block
  %loaded_var3 = load i32, ptr %x, align 4
  %eqcmptmp4 = icmp eq i32 %loaded_var3, 3
  br i1 %eqcmptmp4, label %then_block2, label %else_block

then_block2:                                      ; preds = %elif_block
  store i32 2, ptr %y, align 4
  br label %merge_block

else_block:                                       ; preds = %elif_block
  store i32 3, ptr %y, align 4
  br label %merge_block

merge_block:                                      ; preds = %else_block, %then_block2, %then_block
  %loaded_var5 = load i32, ptr %y, align 4
  store i32 %loaded_var5, ptr %y, align 4
  ret i32 0
}
