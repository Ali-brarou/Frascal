; ModuleID = 'main_module'
source_filename = "main_module"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  %y = alloca float, align 4
  store i32 32, ptr %x, align 4
  %loaded_var = load i32, ptr %x, align 4
  %faddtmp = add i32 32, %loaded_var
  %casted_float = sitofp i32 %faddtmp to float
  store float %casted_float, ptr %y, align 4
  %loaded_var1 = load i32, ptr %x, align 4
  %idivtmp = sdiv i32 %loaded_var1, 3
  store i32 %idivtmp, ptr %x, align 4
  %loaded_var2 = load i32, ptr %x, align 4
  %modtmp = srem i32 %loaded_var2, 3
  %casted_float3 = sitofp i32 %modtmp to float
  store float %casted_float3, ptr %y, align 4
  %loaded_var4 = load i32, ptr %x, align 4
  %casted_float5 = sitofp i32 %loaded_var4 to float
  store float %casted_float5, ptr %y, align 4
  ret i32 0
}
