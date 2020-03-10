contract C {
    uint256 immutable x;
    constructor() public {
        x = 42;
    }
    function f() public view returns (uint256 r) {
        return x;
    }
}
// ----
// f() -> 42