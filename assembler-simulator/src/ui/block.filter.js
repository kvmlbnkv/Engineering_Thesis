app.filter('block', function() {
    return function(input, block) {
        block = +block; //parse to int
        return input.slice(block*256, block*256+256);
    };
});