; ModuleID = 'app.c'
source_filename = "app.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

@app.field = internal global [16384 x i32] zeroinitializer, align 4
@app.nextField = internal global [16384 x i32] zeroinitializer, align 4

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define void @initEmptyField(ptr noundef %0, i32 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  store i32 0, ptr %5, align 4
  br label %6

6:                                                ; preds = %17, %2
  %7 = load i32, ptr %5, align 4
  %8 = load i32, ptr %4, align 4
  %9 = load i32, ptr %4, align 4
  %10 = mul nsw i32 %8, %9
  %11 = icmp slt i32 %7, %10
  br i1 %11, label %12, label %20

12:                                               ; preds = %6
  %13 = load ptr, ptr %3, align 8
  %14 = load i32, ptr %5, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds i32, ptr %13, i64 %15
  store i32 0, ptr %16, align 4
  br label %17

17:                                               ; preds = %12
  %18 = load i32, ptr %5, align 4
  %19 = add nsw i32 %18, 1
  store i32 %19, ptr %5, align 4
  br label %6, !llvm.loop !6

20:                                               ; preds = %6
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define i32 @countNeighbours(i32 noundef %0, i32 noundef %1, ptr noundef %2, i32 noundef %3) #0 {
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca ptr, align 8
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  %11 = alloca i32, align 4
  %12 = alloca i32, align 4
  %13 = alloca i32, align 4
  store i32 %0, ptr %5, align 4
  store i32 %1, ptr %6, align 4
  store ptr %2, ptr %7, align 8
  store i32 %3, ptr %8, align 4
  store i32 0, ptr %9, align 4
  store i32 -1, ptr %10, align 4
  br label %14

14:                                               ; preds = %51, %4
  %15 = load i32, ptr %10, align 4
  %16 = icmp slt i32 %15, 2
  br i1 %16, label %17, label %54

17:                                               ; preds = %14
  %18 = load i32, ptr %5, align 4
  %19 = load i32, ptr %10, align 4
  %20 = add nsw i32 %18, %19
  store i32 %20, ptr %11, align 4
  store i32 -1, ptr %12, align 4
  br label %21

21:                                               ; preds = %47, %17
  %22 = load i32, ptr %12, align 4
  %23 = icmp slt i32 %22, 2
  br i1 %23, label %24, label %50

24:                                               ; preds = %21
  %25 = load i32, ptr %6, align 4
  %26 = load i32, ptr %12, align 4
  %27 = add nsw i32 %25, %26
  store i32 %27, ptr %13, align 4
  %28 = load i32, ptr %5, align 4
  %29 = load i32, ptr %11, align 4
  %30 = icmp eq i32 %28, %29
  br i1 %30, label %31, label %36

31:                                               ; preds = %24
  %32 = load i32, ptr %6, align 4
  %33 = load i32, ptr %13, align 4
  %34 = icmp eq i32 %32, %33
  br i1 %34, label %35, label %36

35:                                               ; preds = %31
  br label %47

36:                                               ; preds = %31, %24
  %37 = load ptr, ptr %7, align 8
  %38 = load i32, ptr %11, align 4
  %39 = load i32, ptr %13, align 4
  %40 = load i32, ptr %8, align 4
  %41 = call i32 @getCellValue(ptr noundef %37, i32 noundef %38, i32 noundef %39, i32 noundef %40)
  %42 = icmp eq i32 %41, 1
  br i1 %42, label %43, label %46

43:                                               ; preds = %36
  %44 = load i32, ptr %9, align 4
  %45 = add nsw i32 %44, 1
  store i32 %45, ptr %9, align 4
  br label %46

46:                                               ; preds = %43, %36
  br label %47

47:                                               ; preds = %46, %35
  %48 = load i32, ptr %12, align 4
  %49 = add nsw i32 %48, 1
  store i32 %49, ptr %12, align 4
  br label %21, !llvm.loop !8

50:                                               ; preds = %21
  br label %51

51:                                               ; preds = %50
  %52 = load i32, ptr %10, align 4
  %53 = add nsw i32 %52, 1
  store i32 %53, ptr %10, align 4
  br label %14, !llvm.loop !9

54:                                               ; preds = %14
  %55 = load i32, ptr %9, align 4
  ret i32 %55
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define internal i32 @getCellValue(ptr noundef %0, i32 noundef %1, i32 noundef %2, i32 noundef %3) #0 {
  %5 = alloca ptr, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store ptr %0, ptr %5, align 8
  store i32 %1, ptr %6, align 4
  store i32 %2, ptr %7, align 4
  store i32 %3, ptr %8, align 4
  %9 = load i32, ptr %6, align 4
  %10 = load i32, ptr %8, align 4
  %11 = add nsw i32 %9, %10
  %12 = load i32, ptr %8, align 4
  %13 = srem i32 %11, %12
  store i32 %13, ptr %6, align 4
  %14 = load i32, ptr %7, align 4
  %15 = load i32, ptr %8, align 4
  %16 = add nsw i32 %14, %15
  %17 = load i32, ptr %8, align 4
  %18 = srem i32 %16, %17
  store i32 %18, ptr %7, align 4
  %19 = load ptr, ptr %5, align 8
  %20 = load i32, ptr %6, align 4
  %21 = load i32, ptr %8, align 4
  %22 = mul nsw i32 %20, %21
  %23 = load i32, ptr %7, align 4
  %24 = add nsw i32 %22, %23
  %25 = sext i32 %24 to i64
  %26 = getelementptr inbounds i32, ptr %19, i64 %25
  %27 = load i32, ptr %26, align 4
  ret i32 %27
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define void @stepLife(i32 noundef %0, ptr noundef %1, ptr noundef %2) #0 {
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca i32, align 4
  %10 = alloca i32, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store ptr %2, ptr %6, align 8
  store i32 0, ptr %7, align 4
  br label %11

11:                                               ; preds = %65, %3
  %12 = load i32, ptr %7, align 4
  %13 = load i32, ptr %4, align 4
  %14 = icmp slt i32 %12, %13
  br i1 %14, label %15, label %68

15:                                               ; preds = %11
  store i32 0, ptr %8, align 4
  br label %16

16:                                               ; preds = %61, %15
  %17 = load i32, ptr %8, align 4
  %18 = load i32, ptr %4, align 4
  %19 = icmp slt i32 %17, %18
  br i1 %19, label %20, label %64

20:                                               ; preds = %16
  %21 = load i32, ptr %8, align 4
  %22 = load i32, ptr %7, align 4
  %23 = load ptr, ptr %5, align 8
  %24 = load i32, ptr %4, align 4
  %25 = call i32 @countNeighbours(i32 noundef %21, i32 noundef %22, ptr noundef %23, i32 noundef %24)
  store i32 %25, ptr %9, align 4
  %26 = load i32, ptr %8, align 4
  %27 = load i32, ptr %4, align 4
  %28 = mul nsw i32 %26, %27
  %29 = load i32, ptr %7, align 4
  %30 = add nsw i32 %28, %29
  store i32 %30, ptr %10, align 4
  %31 = load ptr, ptr %5, align 8
  %32 = load i32, ptr %10, align 4
  %33 = sext i32 %32 to i64
  %34 = getelementptr inbounds i32, ptr %31, i64 %33
  %35 = load i32, ptr %34, align 4
  %36 = icmp eq i32 %35, 1
  br i1 %36, label %37, label %51

37:                                               ; preds = %20
  %38 = load i32, ptr %9, align 4
  %39 = icmp eq i32 %38, 2
  br i1 %39, label %43, label %40

40:                                               ; preds = %37
  %41 = load i32, ptr %9, align 4
  %42 = icmp eq i32 %41, 3
  br label %43

43:                                               ; preds = %40, %37
  %44 = phi i1 [ true, %37 ], [ %42, %40 ]
  %45 = zext i1 %44 to i64
  %46 = select i1 %44, i32 1, i32 0
  %47 = load ptr, ptr %6, align 8
  %48 = load i32, ptr %10, align 4
  %49 = sext i32 %48 to i64
  %50 = getelementptr inbounds i32, ptr %47, i64 %49
  store i32 %46, ptr %50, align 4
  br label %60

51:                                               ; preds = %20
  %52 = load i32, ptr %9, align 4
  %53 = icmp eq i32 %52, 3
  %54 = zext i1 %53 to i64
  %55 = select i1 %53, i32 1, i32 0
  %56 = load ptr, ptr %6, align 8
  %57 = load i32, ptr %10, align 4
  %58 = sext i32 %57 to i64
  %59 = getelementptr inbounds i32, ptr %56, i64 %58
  store i32 %55, ptr %59, align 4
  br label %60

60:                                               ; preds = %51, %43
  br label %61

61:                                               ; preds = %60
  %62 = load i32, ptr %8, align 4
  %63 = add nsw i32 %62, 1
  store i32 %63, ptr %8, align 4
  br label %16, !llvm.loop !10

64:                                               ; preds = %16
  br label %65

65:                                               ; preds = %64
  %66 = load i32, ptr %7, align 4
  %67 = add nsw i32 %66, 1
  store i32 %67, ptr %7, align 4
  br label %11, !llvm.loop !11

68:                                               ; preds = %11
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define void @randomizeField(ptr noundef %0, i32 noundef %1) #0 {
  %3 = alloca ptr, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store ptr %0, ptr %3, align 8
  store i32 %1, ptr %4, align 4
  store i32 0, ptr %5, align 4
  br label %8

8:                                                ; preds = %36, %2
  %9 = load i32, ptr %5, align 4
  %10 = load i32, ptr %4, align 4
  %11 = icmp slt i32 %9, %10
  br i1 %11, label %12, label %39

12:                                               ; preds = %8
  store i32 0, ptr %6, align 4
  br label %13

13:                                               ; preds = %32, %12
  %14 = load i32, ptr %6, align 4
  %15 = load i32, ptr %4, align 4
  %16 = icmp slt i32 %14, %15
  br i1 %16, label %17, label %35

17:                                               ; preds = %13
  %18 = call i32 @simRand()
  %19 = srem i32 %18, 5
  %20 = icmp eq i32 %19, 0
  %21 = zext i1 %20 to i64
  %22 = select i1 %20, i32 1, i32 0
  store i32 %22, ptr %7, align 4
  %23 = load i32, ptr %7, align 4
  %24 = load ptr, ptr %3, align 8
  %25 = load i32, ptr %6, align 4
  %26 = load i32, ptr %4, align 4
  %27 = mul nsw i32 %25, %26
  %28 = load i32, ptr %5, align 4
  %29 = add nsw i32 %27, %28
  %30 = sext i32 %29 to i64
  %31 = getelementptr inbounds i32, ptr %24, i64 %30
  store i32 %23, ptr %31, align 4
  br label %32

32:                                               ; preds = %17
  %33 = load i32, ptr %6, align 4
  %34 = add nsw i32 %33, 1
  store i32 %34, ptr %6, align 4
  br label %13, !llvm.loop !12

35:                                               ; preds = %13
  br label %36

36:                                               ; preds = %35
  %37 = load i32, ptr %5, align 4
  %38 = add nsw i32 %37, 1
  store i32 %38, ptr %5, align 4
  br label %8, !llvm.loop !13

39:                                               ; preds = %8
  ret void
}

declare i32 @simRand() #1

; Function Attrs: noinline nounwind optnone ssp uwtable(sync)
define void @app() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 128, ptr %1, align 4
  call void @initEmptyField(ptr noundef @app.field, i32 noundef 128)
  call void @randomizeField(ptr noundef @app.field, i32 noundef 128)
  store i32 0, ptr %2, align 4
  br label %4

4:                                                ; preds = %23, %0
  %5 = load i32, ptr %2, align 4
  %6 = icmp slt i32 %5, 2000
  br i1 %6, label %7, label %26

7:                                                ; preds = %4
  call void @renderField(i32 noundef 128, ptr noundef @app.field)
  call void @stepLife(i32 noundef 128, ptr noundef @app.field, ptr noundef @app.nextField)
  store i32 0, ptr %3, align 4
  br label %8

8:                                                ; preds = %19, %7
  %9 = load i32, ptr %3, align 4
  %10 = icmp slt i32 %9, 16384
  br i1 %10, label %11, label %22

11:                                               ; preds = %8
  %12 = load i32, ptr %3, align 4
  %13 = sext i32 %12 to i64
  %14 = getelementptr inbounds [16384 x i32], ptr @app.nextField, i64 0, i64 %13
  %15 = load i32, ptr %14, align 4
  %16 = load i32, ptr %3, align 4
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds [16384 x i32], ptr @app.field, i64 0, i64 %17
  store i32 %15, ptr %18, align 4
  br label %19

19:                                               ; preds = %11
  %20 = load i32, ptr %3, align 4
  %21 = add nsw i32 %20, 1
  store i32 %21, ptr %3, align 4
  br label %8, !llvm.loop !14

22:                                               ; preds = %8
  br label %23

23:                                               ; preds = %22
  %24 = load i32, ptr %2, align 4
  %25 = add nsw i32 %24, 1
  store i32 %25, ptr %2, align 4
  br label %4, !llvm.loop !15

26:                                               ; preds = %4
  call void @renderField(i32 noundef 128, ptr noundef @app.field)
  ret void
}

declare void @renderField(i32 noundef, ptr noundef) #1

attributes #0 = { noinline nounwind optnone ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #1 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 5]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 21.1.2"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
!10 = distinct !{!10, !7}
!11 = distinct !{!11, !7}
!12 = distinct !{!12, !7}
!13 = distinct !{!13, !7}
!14 = distinct !{!14, !7}
!15 = distinct !{!15, !7}
