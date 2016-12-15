use Test::Nginx::Socket;

plan tests => repeat_each() * blocks();

run_tests();
__DATA__
=== TEST 1: HEAD
--- config
    location /103 {
        add_early_header "Link" "</main.css>;rel=preload";
    }
--- request
    HEAD /103
--- error_code: 103

=== TEST 2: POST
--- config
    location /103 {
        add_early_header "Link" "</main.css>;rel=preload";
    }
--- request
    POST /103
--- error_code: 405
