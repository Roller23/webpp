# webpp
A very small and simple HTTP web server framework written in C++

Features:

- Easily create simple endpoints for GET and POST requests
- Send files
- Serve static files

Note: as of now there's no security at all. DO NOT use it in serious production environment!

Example usage:

<img width="1275" alt="Screenshot 2021-06-12 at 18 03 55" src="https://user-images.githubusercontent.com/25384028/121782265-06dd2580-cba9-11eb-82b6-5cc8b02d786d.png">

<img width="1274" alt="Screenshot 2021-06-12 at 18 04 10" src="https://user-images.githubusercontent.com/25384028/121782276-178d9b80-cba9-11eb-9ab1-9962351b5fd9.png">

<img width="1240" alt="Screenshot 2021-06-12 at 18 04 32" src="https://user-images.githubusercontent.com/25384028/121782285-1c524f80-cba9-11eb-9403-441c3c59f8ec.png">

TODO:

- Implement multithreading/multiplexing
- Add support for more methods
- Add support for content types other than text/html
- Security improvements
