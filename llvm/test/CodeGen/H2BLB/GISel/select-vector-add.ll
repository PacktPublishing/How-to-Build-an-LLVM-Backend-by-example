define void @scalarizeAndCallLib(ptr %src, ptr %dst) {
  %val = load <2 x i16>, ptr %src
  %res = add <2 x i16> %val, %val
  store <2 x i16> %res, ptr %dst
  ret void
}
