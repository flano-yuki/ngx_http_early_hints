use Test::Nginx::Socket;

plan tests => repeat_each() * blocks();

run_tests();

__DATA__
=== TEST 1: erly_hints
--- config
    location /103 {
        add_early_header "Link" "</main.css>;rel=preload";
        add_early_header "Link" "</main.js>;rel=preload";
    }
--- request
    GET /103
--- error_code: 103

=== TEST 2: no erly_hints
--- config
    location /103 {
        add_early_header "Link" "</main.css>;rel=preload";
        add_early_header "Link" "</main.js>;rel=preload";
    }
--- request
    GET /index.html
--- error_code: 200

=== TEST 3: $early_hints = 1
--- config
    location /103 {
        add_early_header "Link" "</main.css>;rel=preload";
        add_early_header "Link" "</main.js>;rel=preload";
        if ($early_hints = 1) {
           return 200;
        }
    }
--- request
    GET /103
--- error_code: 200

=== TEST 4: $early_hints = 0
--- config
    location /no-103 {
        if ($early_hints = 0) {
           return 200;
        }
    }
--- request
    GET /no-103
--- error_code: 200
