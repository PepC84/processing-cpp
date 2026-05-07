/**
 * Additive Wave
 * by Daniel Shiffman. 
 * 
 * Create a more complex wave by adding two waves together. 
 */
 
int xspacing = 8;   // How far apart each horizontal location is spaced
int w;              // Width of entire wave
int maxwaves = 4;   // total # of waves

float theta = 0.0f;

std::vector<float> amplitude;   // Height of wave
std::vector<float> dx;          // Increment per x
std::vector<float> yvalues;     // Final wave values

void setup() {
  size(640, 360);
  frameRate(30);
  colorMode(RGB, 255, 255, 255, 100);

  w = width + 16;

  amplitude.resize(maxwaves);
  dx.resize(maxwaves);

  for (int i = 0; i < maxwaves; i++) {
    amplitude[i] = random(10, 30);

    float period = random(100, 300);
    dx[i] = (TWO_PI / period) * xspacing;
  }

  yvalues.resize(w / xspacing);
}

void draw() {
  background(0);
  calcWave();
  renderWave();
}

void calcWave() {
  // Increment theta
  theta += 0.02f;

  // Reset all heights
  for (int i = 0; i < yvalues.size(); i++) {
    yvalues[i] = 0;
  }

  // Build wave
  for (int j = 0; j < maxwaves; j++) {
    float x = theta;

    for (int i = 0; i < yvalues.size(); i++) {
      if (j % 2 == 0)
        yvalues[i] += sin(x) * amplitude[j];
      else
        yvalues[i] += cos(x) * amplitude[j];

      x += dx[j];
    }
  }
}

void renderWave() {
  noStroke();
  fill(255, 50);
  ellipseMode(CENTER);

  for (int x = 0; x < yvalues.size(); x++) {
    ellipse(x * xspacing,
            height / 2 + yvalues[x],
            16, 16);
  }
}
