; ModuleID = 'app.c'
source_filename = "app.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

@app.field = internal global [16384 x i32] zeroinitializer, align 4
@app.nextField = internal unnamed_addr global [16384 x i32] zeroinitializer, align 4

; Function Attrs: mustprogress nofree norecurse nosync nounwind ssp willreturn memory(argmem: write) uwtable(sync)
define void @initEmptyField(ptr noundef writeonly captures(none) %0, i32 noundef %1) local_unnamed_addr #0 {
  %3 = icmp eq i32 %1, 0
  br i1 %3, label %9, label %4

4:                                                ; preds = %2
  %5 = mul i32 %1, %1
  %6 = tail call i32 @llvm.umax.i32(i32 %5, i32 1)
  %7 = zext i32 %6 to i64
  %8 = shl nuw nsw i64 %7, 2
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(1) %0, i8 0, i64 %8, i1 false), !tbaa !6
  br label %9

9:                                                ; preds = %4, %2
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind ssp memory(argmem: read) uwtable(sync)
define i32 @countNeighbours(i32 noundef %0, i32 noundef %1, ptr noundef readonly captures(none) %2, i32 noundef %3) local_unnamed_addr #1 {
  %5 = add i32 %3, %0
  %6 = add i32 %3, %1
  br label %8

7:                                                ; preds = %12
  ret i32 %33

8:                                                ; preds = %4, %12
  %9 = phi i32 [ 0, %4 ], [ %33, %12 ]
  %10 = phi i32 [ -1, %4 ], [ %13, %12 ]
  %11 = add i32 %5, %10
  br label %15

12:                                               ; preds = %32
  %13 = add nsw i32 %10, 1
  %14 = icmp eq i32 %13, 2
  br i1 %14, label %7, label %8, !llvm.loop !10

15:                                               ; preds = %8, %32
  %16 = phi i32 [ %9, %8 ], [ %33, %32 ]
  %17 = phi i32 [ -1, %8 ], [ %34, %32 ]
  %18 = or i32 %17, %10
  %19 = icmp eq i32 %18, 0
  br i1 %19, label %32, label %20

20:                                               ; preds = %15
  %21 = srem i32 %11, %3
  %22 = add i32 %6, %17
  %23 = srem i32 %22, %3
  %24 = mul nsw i32 %21, %3
  %25 = add nsw i32 %24, %23
  %26 = sext i32 %25 to i64
  %27 = getelementptr inbounds i32, ptr %2, i64 %26
  %28 = load i32, ptr %27, align 4, !tbaa !6
  %29 = icmp eq i32 %28, 1
  %30 = zext i1 %29 to i32
  %31 = add nsw i32 %16, %30
  br label %32

32:                                               ; preds = %15, %20
  %33 = phi i32 [ %31, %20 ], [ %16, %15 ]
  %34 = add nsw i32 %17, 1
  %35 = icmp eq i32 %34, 2
  br i1 %35, label %12, label %15, !llvm.loop !12
}

; Function Attrs: nofree norecurse nosync nounwind ssp memory(argmem: readwrite) uwtable(sync)
define void @stepLife(i32 noundef %0, ptr noundef readonly captures(none) %1, ptr noundef writeonly captures(none) %2) local_unnamed_addr #2 {
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %13

5:                                                ; preds = %3
  %6 = zext nneg i32 %0 to i64
  %7 = zext nneg i32 %0 to i64
  %8 = zext nneg i32 %0 to i64
  br label %9

9:                                                ; preds = %5, %14
  %10 = phi i64 [ 0, %5 ], [ %15, %14 ]
  %11 = trunc i64 %10 to i32
  %12 = add i32 %0, %11
  br label %17

13:                                               ; preds = %14, %3
  ret void

14:                                               ; preds = %49
  %15 = add nuw nsw i64 %10, 1
  %16 = icmp eq i64 %15, %7
  br i1 %16, label %13, label %9, !llvm.loop !13

17:                                               ; preds = %9, %49
  %18 = phi i64 [ 0, %9 ], [ %61, %49 ]
  %19 = trunc i64 %18 to i32
  %20 = add i32 %0, %19
  br label %21

21:                                               ; preds = %25, %17
  %22 = phi i32 [ 0, %17 ], [ %46, %25 ]
  %23 = phi i32 [ -1, %17 ], [ %26, %25 ]
  %24 = add i32 %20, %23
  br label %28

25:                                               ; preds = %45
  %26 = add nsw i32 %23, 1
  %27 = icmp eq i32 %26, 2
  br i1 %27, label %49, label %21, !llvm.loop !10

28:                                               ; preds = %45, %21
  %29 = phi i32 [ %22, %21 ], [ %46, %45 ]
  %30 = phi i32 [ -1, %21 ], [ %47, %45 ]
  %31 = or i32 %30, %23
  %32 = icmp eq i32 %31, 0
  br i1 %32, label %45, label %33

33:                                               ; preds = %28
  %34 = srem i32 %24, %0
  %35 = add i32 %12, %30
  %36 = srem i32 %35, %0
  %37 = mul nsw i32 %34, %0
  %38 = add nsw i32 %37, %36
  %39 = sext i32 %38 to i64
  %40 = getelementptr inbounds i32, ptr %1, i64 %39
  %41 = load i32, ptr %40, align 4, !tbaa !6
  %42 = icmp eq i32 %41, 1
  %43 = zext i1 %42 to i32
  %44 = add nsw i32 %29, %43
  br label %45

45:                                               ; preds = %33, %28
  %46 = phi i32 [ %44, %33 ], [ %29, %28 ]
  %47 = add nsw i32 %30, 1
  %48 = icmp eq i32 %47, 2
  br i1 %48, label %25, label %28, !llvm.loop !12

49:                                               ; preds = %25
  %50 = mul nuw nsw i64 %18, %6
  %51 = add nuw nsw i64 %50, %10
  %52 = getelementptr inbounds nuw i32, ptr %1, i64 %51
  %53 = load i32, ptr %52, align 4, !tbaa !6
  %54 = icmp eq i32 %53, 1
  %55 = icmp eq i32 %46, 3
  %56 = and i32 %46, -2
  %57 = icmp eq i32 %56, 2
  %58 = select i1 %54, i1 %57, i1 %55
  %59 = zext i1 %58 to i32
  %60 = getelementptr inbounds nuw i32, ptr %2, i64 %51
  store i32 %59, ptr %60, align 4, !tbaa !6
  %61 = add nuw nsw i64 %18, 1
  %62 = icmp eq i64 %61, %8
  br i1 %62, label %14, label %17, !llvm.loop !14
}

; Function Attrs: nounwind ssp uwtable(sync)
define void @randomizeField(ptr noundef writeonly captures(none) %0, i32 noundef %1) local_unnamed_addr #3 {
  %3 = icmp sgt i32 %1, 0
  br i1 %3, label %4, label %11

4:                                                ; preds = %2
  %5 = zext nneg i32 %1 to i64
  %6 = zext nneg i32 %1 to i64
  %7 = zext nneg i32 %1 to i64
  br label %8

8:                                                ; preds = %4, %12
  %9 = phi i64 [ 0, %4 ], [ %13, %12 ]
  %10 = getelementptr inbounds i32, ptr %0, i64 %9
  br label %15

11:                                               ; preds = %12, %2
  ret void

12:                                               ; preds = %15
  %13 = add nuw nsw i64 %9, 1
  %14 = icmp eq i64 %13, %6
  br i1 %14, label %11, label %8, !llvm.loop !15

15:                                               ; preds = %8, %15
  %16 = phi i64 [ 0, %8 ], [ %23, %15 ]
  %17 = tail call i32 @simRand() #8
  %18 = srem i32 %17, 5
  %19 = icmp eq i32 %18, 0
  %20 = zext i1 %19 to i32
  %21 = mul nuw nsw i64 %16, %5
  %22 = getelementptr inbounds i32, ptr %10, i64 %21
  store i32 %20, ptr %22, align 4, !tbaa !6
  %23 = add nuw nsw i64 %16, 1
  %24 = icmp eq i64 %23, %7
  br i1 %24, label %12, label %15, !llvm.loop !16
}

declare i32 @simRand() local_unnamed_addr #4

; Function Attrs: nounwind ssp uwtable(sync)
define void @app() local_unnamed_addr #3 {
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 4 dereferenceable(65536) @app.field, i8 0, i64 65536, i1 false), !tbaa !6
  br label %1

1:                                                ; preds = %4, %0
  %2 = phi i64 [ 0, %0 ], [ %5, %4 ]
  %3 = getelementptr inbounds nuw i32, ptr @app.field, i64 %2
  br label %7

4:                                                ; preds = %7
  %5 = add nuw nsw i64 %2, 1
  %6 = icmp eq i64 %5, 128
  br i1 %6, label %18, label %1, !llvm.loop !15

7:                                                ; preds = %7, %1
  %8 = phi i64 [ 0, %1 ], [ %15, %7 ]
  %9 = tail call i32 @simRand() #8
  %10 = srem i32 %9, 5
  %11 = icmp eq i32 %10, 0
  %12 = zext i1 %11 to i32
  %13 = shl nuw nsw i64 %8, 9
  %14 = getelementptr inbounds nuw i8, ptr %3, i64 %13
  store i32 %12, ptr %14, align 4, !tbaa !6
  %15 = add nuw nsw i64 %8, 1
  %16 = icmp eq i64 %15, 128
  br i1 %16, label %4, label %7, !llvm.loop !16

17:                                               ; preds = %26
  tail call void @renderField(i32 noundef 128, ptr noundef nonnull @app.field) #8
  ret void

18:                                               ; preds = %4, %26
  %19 = phi i32 [ %27, %26 ], [ 0, %4 ]
  tail call void @renderField(i32 noundef 128, ptr noundef nonnull @app.field) #8
  br label %20

20:                                               ; preds = %23, %18
  %21 = phi i64 [ 0, %18 ], [ %24, %23 ]
  %22 = trunc i64 %21 to i32
  br label %29

23:                                               ; preds = %60
  %24 = add nuw nsw i64 %21, 1
  %25 = icmp eq i64 %24, 128
  br i1 %25, label %26, label %20, !llvm.loop !13

26:                                               ; preds = %23
  tail call void @llvm.memcpy.p0.p0.i64(ptr noundef nonnull align 4 dereferenceable(65536) @app.field, ptr noundef nonnull align 4 dereferenceable(65536) @app.nextField, i64 65536, i1 false), !tbaa !6
  %27 = add nuw nsw i32 %19, 1
  %28 = icmp eq i32 %27, 2000
  br i1 %28, label %17, label %18, !llvm.loop !17

29:                                               ; preds = %60, %20
  %30 = phi i64 [ 0, %20 ], [ %72, %60 ]
  %31 = trunc i64 %30 to i32
  br label %32

32:                                               ; preds = %38, %29
  %33 = phi i32 [ 0, %29 ], [ %57, %38 ]
  %34 = phi i32 [ -1, %29 ], [ %39, %38 ]
  %35 = add i32 %34, %31
  %36 = shl i32 %35, 7
  %37 = and i32 %36, 16256
  br label %41

38:                                               ; preds = %56
  %39 = add nsw i32 %34, 1
  %40 = icmp eq i32 %39, 2
  br i1 %40, label %60, label %32, !llvm.loop !10

41:                                               ; preds = %56, %32
  %42 = phi i32 [ %33, %32 ], [ %57, %56 ]
  %43 = phi i32 [ -1, %32 ], [ %58, %56 ]
  %44 = or i32 %43, %34
  %45 = icmp eq i32 %44, 0
  br i1 %45, label %56, label %46

46:                                               ; preds = %41
  %47 = add i32 %43, %22
  %48 = and i32 %47, 127
  %49 = or disjoint i32 %48, %37
  %50 = zext nneg i32 %49 to i64
  %51 = getelementptr inbounds nuw i32, ptr @app.field, i64 %50
  %52 = load i32, ptr %51, align 4, !tbaa !6
  %53 = icmp eq i32 %52, 1
  %54 = zext i1 %53 to i32
  %55 = add nsw i32 %42, %54
  br label %56

56:                                               ; preds = %46, %41
  %57 = phi i32 [ %55, %46 ], [ %42, %41 ]
  %58 = add nsw i32 %43, 1
  %59 = icmp eq i32 %58, 2
  br i1 %59, label %38, label %41, !llvm.loop !12

60:                                               ; preds = %38
  %61 = shl nuw nsw i64 %30, 7
  %62 = add nuw nsw i64 %61, %21
  %63 = getelementptr inbounds nuw i32, ptr @app.field, i64 %62
  %64 = load i32, ptr %63, align 4, !tbaa !6
  %65 = icmp eq i32 %64, 1
  %66 = icmp eq i32 %57, 3
  %67 = and i32 %57, -2
  %68 = icmp eq i32 %67, 2
  %69 = select i1 %65, i1 %68, i1 %66
  %70 = zext i1 %69 to i32
  %71 = getelementptr inbounds nuw i32, ptr @app.nextField, i64 %62
  store i32 %70, ptr %71, align 4, !tbaa !6
  %72 = add nuw nsw i64 %30, 1
  %73 = icmp eq i64 %72, 128
  br i1 %73, label %23, label %29, !llvm.loop !14
}

declare void @renderField(i32 noundef, ptr noundef) local_unnamed_addr #4

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias writeonly captures(none), ptr noalias readonly captures(none), i64, i1 immarg) #7

attributes #0 = { mustprogress nofree norecurse nosync nounwind ssp willreturn memory(argmem: write) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #1 = { nofree norecurse nosync nounwind ssp memory(argmem: read) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #2 = { nofree norecurse nosync nounwind ssp memory(argmem: readwrite) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #3 = { nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #4 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #8 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 15, i32 5]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Homebrew clang version 21.1.2"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.unroll.disable"}
!12 = distinct !{!12, !11}
!13 = distinct !{!13, !11}
!14 = distinct !{!14, !11}
!15 = distinct !{!15, !11}
!16 = distinct !{!16, !11}
!17 = distinct !{!17, !11}
