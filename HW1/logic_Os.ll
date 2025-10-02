; ModuleID = 'logic.c'
source_filename = "logic.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx15.0.0"

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize ssp willreturn memory(argmem: write) uwtable(sync)
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

; Function Attrs: nofree norecurse nosync nounwind optsize ssp memory(argmem: read) uwtable(sync)
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
  br i1 %14, label %7, label %8

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
  br i1 %35, label %12, label %15
}

; Function Attrs: nofree norecurse nosync nounwind optsize ssp memory(argmem: readwrite) uwtable(sync)
define void @stepLife(i32 noundef %0, ptr noundef readonly captures(none) %1, ptr noundef writeonly captures(none) %2) local_unnamed_addr #2 {
  %4 = icmp sgt i32 %0, 0
  br i1 %4, label %5, label %10

5:                                                ; preds = %3
  %6 = zext nneg i32 %0 to i64
  br label %7

7:                                                ; preds = %11, %5
  %8 = phi i64 [ 0, %5 ], [ %12, %11 ]
  %9 = trunc nuw nsw i64 %8 to i32
  br label %14

10:                                               ; preds = %11, %3
  ret void

11:                                               ; preds = %14
  %12 = add nuw nsw i64 %8, 1
  %13 = icmp eq i64 %12, %6
  br i1 %13, label %10, label %7

14:                                               ; preds = %7, %14
  %15 = phi i64 [ 0, %7 ], [ %29, %14 ]
  %16 = trunc nuw nsw i64 %15 to i32
  %17 = tail call i32 @countNeighbours(i32 noundef %16, i32 noundef %9, ptr noundef %1, i32 noundef %0) #7
  %18 = mul nuw nsw i64 %15, %6
  %19 = add nuw nsw i64 %18, %8
  %20 = getelementptr inbounds nuw i32, ptr %1, i64 %19
  %21 = load i32, ptr %20, align 4, !tbaa !6
  %22 = icmp eq i32 %21, 1
  %23 = and i32 %17, -2
  %24 = icmp eq i32 %23, 2
  %25 = icmp eq i32 %17, 3
  %26 = select i1 %22, i1 %24, i1 %25
  %27 = zext i1 %26 to i32
  %28 = getelementptr inbounds nuw i32, ptr %2, i64 %19
  store i32 %27, ptr %28, align 4, !tbaa !6
  %29 = add nuw nsw i64 %15, 1
  %30 = icmp eq i64 %29, %6
  br i1 %30, label %11, label %14
}

; Function Attrs: nounwind optsize ssp uwtable(sync)
define void @randomizeField(ptr noundef writeonly captures(none) %0, i32 noundef %1) local_unnamed_addr #3 {
  %3 = icmp sgt i32 %1, 0
  br i1 %3, label %4, label %9

4:                                                ; preds = %2
  %5 = zext nneg i32 %1 to i64
  br label %6

6:                                                ; preds = %10, %4
  %7 = phi i64 [ 0, %4 ], [ %11, %10 ]
  %8 = getelementptr inbounds nuw i32, ptr %0, i64 %7
  br label %13

9:                                                ; preds = %10, %2
  ret void

10:                                               ; preds = %13
  %11 = add nuw nsw i64 %7, 1
  %12 = icmp eq i64 %11, %5
  br i1 %12, label %9, label %6

13:                                               ; preds = %6, %13
  %14 = phi i64 [ 0, %6 ], [ %21, %13 ]
  %15 = tail call i32 @simRand() #8
  %16 = srem i32 %15, 5
  %17 = icmp eq i32 %16, 0
  %18 = zext i1 %17 to i32
  %19 = mul nuw nsw i64 %14, %5
  %20 = getelementptr inbounds nuw i32, ptr %8, i64 %19
  store i32 %18, ptr %20, align 4, !tbaa !6
  %21 = add nuw nsw i64 %14, 1
  %22 = icmp eq i64 %21, %5
  br i1 %22, label %10, label %13
}

; Function Attrs: optsize
declare i32 @simRand() local_unnamed_addr #4

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr writeonly captures(none), i8, i64, i1 immarg) #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i32 @llvm.umax.i32(i32, i32) #6

attributes #0 = { mustprogress nofree norecurse nosync nounwind optsize ssp willreturn memory(argmem: write) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #1 = { nofree norecurse nosync nounwind optsize ssp memory(argmem: read) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #2 = { nofree norecurse nosync nounwind optsize ssp memory(argmem: readwrite) uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #3 = { nounwind optsize ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #4 = { optsize "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8a" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #7 = { optsize }
attributes #8 = { nounwind optsize }

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
