Bubble[] bubbles;
JSONObject json;

void setup() {
    size(640, 360);
    loadData();
}

void draw() {

    background(255);

    if (bubbles != null) {

        for (Bubble b : bubbles) {
            b.display();
            b.rollover(mouseX, mouseY);
        }
    }

    fill(0);
    textAlign(LEFT);
    text("Click to add bubbles.", 10, height - 10);
}

void loadData() {

    json = loadJSONObject("data.json");

    JSONArray bubbleData = json.getJSONArray("bubbles");

    bubbles = new Bubble[bubbleData.size()];

    for (int i = 0; i < bubbleData.size(); i++) {

        JSONObject bubble = bubbleData.getJSONObject(i);

        JSONObject position = bubble.getJSONObject("position");

        float x = position.getFloat("x");
        float y = position.getFloat("y");

        float diameter = bubble.getFloat("diameter");
        String label = bubble.getString("label");

        bubbles[i] = new Bubble(x, y, diameter, label);
    }
}

void mousePressed() {

    JSONObject newBubble = new JSONObject();

    JSONObject position = new JSONObject();

    position.setFloat("x", mouseX);
    position.setFloat("y", mouseY);

    newBubble.setJSONObject("position", position);

    newBubble.setFloat("diameter", random(40, 80));
    newBubble.setString("label", "New label");

    JSONArray bubbleData = json.getJSONArray("bubbles");

    bubbleData.append(newBubble);

    if (bubbleData.size() > 10) {
        bubbleData.remove(0);
    }

    saveJSONObject(json, "data/data.json");

    loadData();
}

class Bubble {

    float x, y;
    float diameter;
    String name;

    boolean over = false;

    Bubble(float x_, float y_, float diameter_, String name_) {

        x = x_;
        y = y_;
        diameter = diameter_;
        name = name_;
    }

    void rollover(float px, float py) {

        float d = dist(px, py, x, y);

        over = (d < diameter / 2);
    }

    void display() {

        stroke(0);
        strokeWeight(2);
        noFill();

        ellipse(x, y, diameter, diameter);

        if (over) {

            fill(0);
            textAlign(CENTER);

            text(name, x, y + diameter / 2 + 20);
        }
    }
}
