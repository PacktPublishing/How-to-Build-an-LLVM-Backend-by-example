define void @clampi3(ptr %src, ptr %dst) {
  %val = load i3, ptr %src
  %res = add i3 %val, 2
  store i3 %res, ptr %dst
  ret void
}
