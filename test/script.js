document.getElementById("button").onclick = function() {
  for (let i = 0; i < 10; i++) {
    document.body.innerHTML += ` I am added from javascript ${i}`;
  }
}
