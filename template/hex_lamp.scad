$fn=75;

a = 86.567;    // Side length on outer (larger) face
hex_rad = sqrt(3) * a / 2; // outer radius of hexagon
t_sheet = 6.2;  // Sheet thickness

r_nipple = 4;
nippel_dist = 9;

// height of lamp when standing on a hexagon face
hexa_height = sqrt(6) * a;
echo("Hexa_height: ", hexa_height);

// height of lamp when standing on a square face
square_height = 2 * sqrt(2) * a;

// Inside angles between the N sided pieces
alpha_4_6 = acos(-1 / sqrt(3));
alpha_6_6 = acos(-1 / 3);

// to move on x and y from outer shell to inner shell. Don't ask how!!
function magic_dist(t_sheet) = 0.517637 * t_sheet;

module move_inside(t_sheet) {
	// Move from the outside wall to the inside wall
	translate([magic_dist(t_sheet), magic_dist(t_sheet), -t_sheet]) children();
}

// 3D shape of the square piece. Need 6 of these for complete shape
module squarepiece(t_sheet, holes=true) {
	module tempHole() {
		translate([nippel_dist / sqrt(2) + magic_dist(t_sheet), -nippel_dist / sqrt(2) - magic_dist(t_sheet), 0])
			translate([-a / 2, a / 2, 0])
				translate([0, 0, -1]) cylinder(h=t_sheet + 2, r=r_nipple, center=false);
	}
	translate([0, 0, -t_sheet]) difference() {
		translate([0, 0, t_sheet / 2]) cube(size=[a, a, t_sheet], center=true);
		// Take the 4 edges off at alpha_4_6
		for (alpha=[0:90:270]) {
			cut_angle = (180 - alpha_4_6) / 2;
			cut_dist = t_sheet * tan(cut_angle);
			rotate([0, 0, alpha])
				translate([0, -a / 2 + cut_dist, 0])
					rotate([cut_angle, 0, 0])
						translate([0, -t_sheet, t_sheet])
							cube(size=[a + t_sheet * 2, t_sheet * 2, t_sheet * 2], center=true);
		}
		// Drill the 4 holes
		if (holes) {
			tempHole();
			mirror([1, 0, 0]) tempHole();
			mirror([0, 1, 0]){
				tempHole();
				mirror([1, 0, 0]) tempHole();
			}
		}
	}
}

// 3D shape of the hexagonal piece. Need 8 of these for complete shape
module hexpiece(t_sheet, holes=true) {
	fn = 6;
	fudge = 1 / cos(180 / fn);

	module tempHole() {
		translate([-a / 2, -hex_rad, -1])
			translate([magic_dist(t_sheet), magic_dist(t_sheet), 0])
				rotate([0, 0, 60]) translate([nippel_dist, 0, 0])
					cylinder(h=t_sheet + 2, r=r_nipple);
	}

	translate([0, 0, -t_sheet]) difference() {
		cylinder(h=t_sheet, r=hex_rad * fudge, $fn=fn);

		// Take the 6 edges off
		// alternating at the 4_6 and 6_6 angle
		for (alpha=[0:60:300]) {
			tmp = (alpha % 120) == 0 ? alpha_4_6 : alpha_6_6;
			cut_angle = (180 - tmp) / 2;
			cut_dist = t_sheet * tan(cut_angle);
			rotate([0, 0, alpha])
				translate([0, -hex_rad + cut_dist, 0])
					rotate([cut_angle, 0, 0])
						translate([0, -t_sheet, t_sheet])
							cube(size=[a + t_sheet * 2, t_sheet * 2, t_sheet * 2], center=true);
		}

		// Drill the 6 holes, with thickness dependent offset
		if (holes) {
			for (alpha=[0:120:240]) {
				rotate([0, 0, alpha]) tempHole();
				rotate([0, 0, alpha]) mirror([1, 0, 0]) tempHole();
			}
		}
	}
}

//  put 3 side pieces together
module corner_piece(t_sheet=30, holes=true) {
	union() {
		translate([a / 2 - 0.001, hex_rad - 0.001, 0])
			hexpiece(t_sheet, holes);

		rotate([(180 - alpha_4_6), 0, 0])
			translate([a / 2, -a / 2, 0])
				color("green") squarepiece(t_sheet, holes);

		rotate([180 + alpha_6_6, 0, 120])
			translate([a / 2, hex_rad, 0])
				rotate([0, 0, 60])
					color("blue") hexpiece(t_sheet, holes);
	}
}

// one nipple, centered on origin
module nipple_piece(n_r, n_h)
{
	difference() {
		translate([0, 0, n_h / 2]) cylinder(h=n_h, r=n_r, center=true);
		// Cut outs
		cube(size=[11, 0.5, 150], center=true);
		cube(size=[0.5, 11, 150], center=true);
	}
}

// operation to create the 3 nipples with same angle as the 3 corner faces
// nippel_dist is distance from top corner along the face
module position_nipples(nippel_dist)
{
	union() {
		rotate([0, 0, 60])
			translate([nippel_dist, 0, 0])
				children(0);
		rotate([(180 - alpha_4_6), 0, 0])
			rotate([0, 0, -45])
				translate([nippel_dist, 0, 0])
					children(0);
		rotate([180 + alpha_6_6, 0, 120])
			rotate([0, 0, 60])
				translate([nippel_dist, 0, 0])
						children(0);
	}
}

// 3 thick side pieces + 3 nipples
module corner_piece_with_nipples() {
	difference() {
		union() {
			corner_piece(30, false);
			position_nipples(nippel_dist) nipple_piece(r_nipple, t_sheet);
		}
		// Central screw hole through nippel
		position_nipples(nippel_dist)
			cylinder(h=3 * t_sheet, r=3.75 / 2, center=true);
	}
}

// Create the joiner piece by intersecting the above with a cube
module joiner() {
	intersection() {
		// angles fine tuned to get similar area on faces
		corner_piece_with_nipples();
		// translate([0, 0, 70]) sphere(r=80);
		rotate([32, -23, 0]) translate([0, 0, 41]) cube([200, 200, 100], true);
	}
}

// -----------------------------
//  Cross-check the joiners fit
// -----------------------------
// t_sheet = 35;
module tempjoin() {translate([-a / 2, -hex_rad, 0]) move_inside(t_sheet) joiner();}
translate([-a / 2, -hex_rad, 0]) corner_piece(t_sheet);
	for (alpha=[0:120:240]) {
		rotate([0, 0, alpha]) union() {
			tempjoin();
			mirror([1, 0, 0]) tempjoin();
		}
	}

// corner_piece(30, true);

// hexpiece(t_sheet, true);


//-----------------
// Export files
//-----------------
// [joiner.stl]
// translate([0, 0, 9]) rotate([-32, 0, 0]) rotate([0, 23, 0]) joiner();

// [hex.svg]
// projection(cut=true)
// 	translate([0, 0, 0.000001])
// 		hexpiece(t_sheet);

// [square.svg]
// projection(cut=true)
// 	translate([0, 0, 0.000001])
// 		squarepiece(t_sheet);

