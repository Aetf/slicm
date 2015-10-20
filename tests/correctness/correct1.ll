; ModuleID = 'correct1.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common global [100 x i32] zeroinitializer, align 16
@b = common global [100 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [12 x i8] c"%d, %d, %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %x = alloca i32, align 4
  %i = alloca i32, align 4
  %ptra = alloca i32*, align 8
  store i32 0, i32* %1, align 4
  store i32 0, i32* %x, align 4
  store i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i32 0, i32 0), i32** %ptra, align 8
  store i32 0, i32* %i, align 4
  br label %2

; <label>:2                                       ; preds = %10, %0
  %3 = load i32, i32* %i, align 4
  %4 = icmp slt i32 %3, 100
  br i1 %4, label %5, label %13

; <label>:5                                       ; preds = %2
  %6 = load i32, i32* %i, align 4
  %7 = load i32, i32* %i, align 4
  %8 = sext i32 %7 to i64
  %9 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %8
  store i32 %6, i32* %9, align 4
  br label %10

; <label>:10                                      ; preds = %5
  %11 = load i32, i32* %i, align 4
  %12 = add nsw i32 %11, 1
  store i32 %12, i32* %i, align 4
  br label %2

; <label>:13                                      ; preds = %2
  store i32 0, i32* %i, align 4
  br label %14

; <label>:14                                      ; preds = %30, %13
  %15 = load i32, i32* %i, align 4
  %16 = icmp slt i32 %15, 100
  br i1 %16, label %17, label %33

; <label>:17                                      ; preds = %14
  %18 = load i32, i32* %i, align 4
  %19 = icmp sgt i32 %18, 999
  br i1 %19, label %20, label %25

; <label>:20                                      ; preds = %17
  %21 = load i32, i32* %i, align 4
  %22 = add nsw i32 %21, 1
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [100 x i32], [100 x i32]* @a, i64 0, i64 %23
  store i32 1, i32* %24, align 4
  br label %25

; <label>:25                                      ; preds = %20, %17
  %26 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @a, i64 0, i64 97), align 4
  %27 = load i32, i32* %i, align 4
  %28 = sext i32 %27 to i64
  %29 = getelementptr inbounds [100 x i32], [100 x i32]* @b, i64 0, i64 %28
  store i32 %26, i32* %29, align 4
  br label %30

; <label>:30                                      ; preds = %25
  %31 = load i32, i32* %i, align 4
  %32 = add nsw i32 %31, 1
  store i32 %32, i32* %i, align 4
  br label %14

; <label>:33                                      ; preds = %14
  %34 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @b, i64 0, i64 97), align 4
  %35 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @b, i64 0, i64 98), align 8
  %36 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @b, i64 0, i64 99), align 4
  %37 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i32 0, i32 0), i32 %34, i32 %35, i32 %36)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk)"}
