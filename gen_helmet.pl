use v5.10;
$pi = 4*atan2(1, 1);

for ($a = $ARGV[0]/180*$pi; $a <= ($ARGV[0]+180)/180*$pi; $a += $pi/7) {
	say "\tb2Vec2(", eval($ARGV[1]) + 7.5/16*cos($a), ", ", eval($ARGV[2]) + 7.5/16*sin($a), "),";
}
