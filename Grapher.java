import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;

public class Grapher {
  protected InputStream input_stream;

  public static void main(String args[]) {}

  Grapher() {}
}

class Frame {
  public class Label {
    double y;
    int value;
  }

  public class Point {
    double x, y;
  }

  public Label labels[];
  public Point points[];

  // Reads the next frame from the InputStream. Returns null if no more frames
  // are available.
  static Frame ReadNextFrame(DataInputStream in) throws Exception {
    int num_labels = Integer.reverseBytes(in.readInt());
    int num_points = Integer.reverseBytes(in.readInt());
    labels = new Label[num_labels];
    points = new Point[num_points];
    for (int i = 0; i < labels.length; ++i) {
      labels[i].y = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
      labels[i].value = Integer.reverseBytes(in.readInt());
    }
    for (int i = 0; i < points.length; ++i) {
      points[i].x = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
      points[i].y = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
    }
    return new Frame(labels, points);
  }

  protected Frame(Label labels_arg[], Points points_arg[]) {
    labels = labels_arg;
    points = points_arg;
  }
}
