import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.*;
import javax.swing.*;

public class Grapher extends JPanel {
  protected Frame frames_[];
  protected Image buffer_;
  private static final int kPadding = 5;
  protected AtomicInteger current_frame_ = new AtomicInteger(0);
  protected long start_drawing_at_ = 0;

  public static void main(String args[]) throws Exception {
    JFrame frame = new JFrame("grapher");

    Grapher grapher = new Grapher();
    System.err.printf("done creating grapher.\n");
    Container c = frame.getContentPane();
    c.setLayout(new FlowLayout());
    c.add(grapher);
    frame.pack();
    System.err.printf("packed: %s.\n", grapher.getBounds());
    frame.setVisible(true);
    grapher.Go();
  }

  public Grapher() throws Exception {
    DataInputStream in =
        new DataInputStream(
            new BufferedInputStream(
                new FileInputStream("time_series.out")));

    Vector<Frame> frames = new Vector<Frame>();
    Frame frame;
    while ((frame = Frame.ReadNextFrame(in)) != null) {
      frames.add(frame);
      System.err.printf("\r%d", frames.size());
    }
    frames_ = frames.toArray(new Frame[0]);
    System.err.printf("\n");
    in.close();
    System.err.printf("Read %d frames.\n", frames_.length);
  }

  private void Go() {
    start_drawing_at_ = System.currentTimeMillis();
    new Thread(new UpdaterRunner()).start();
  }

  private void Update() {
    int fps = 100;
    int frame = (int)((System.currentTimeMillis() - start_drawing_at_) / 1000.0 * fps);
    if (current_frame_.get() != frame) {
      current_frame_.set(frame);
    }
    repaint();
  }

  private class UpdaterRunner implements Runnable {
    public void UpdaterRunner() {}
    public void run() {
      for (;;) {
        try {
          Thread.sleep(1);
          Update();
        } catch (Exception e){}
      }
    }
  }

  public Dimension getPreferredSize() {
    return new Dimension(500, 500);
  }

  public Dimension getMinimumSize() {
    return new Dimension(500, 500);
  }

  public Dimension getMaximumSize() {
    return new Dimension(500, 500);
  }

  public void RenderFrame(int index, Graphics2D g2d) {
    Frame frame = frames_[index];

    int kPadding = 40;
    int full_width = getWidth();
    int width = getWidth() - kPadding, height = getHeight();
    g2d.setColor(Color.BLACK);
    g2d.fillRect(0, 0, full_width, height);
    g2d.setColor(Color.WHITE);
    for (Frame.Label l : frame.labels) {
      int y = height - (int)(l.y * height);
      g2d.drawString(l.value + "", 0, y);
    }
    g2d.setTransform(AffineTransform.getTranslateInstance(kPadding, 0));
    g2d.setColor(Color.LIGHT_GRAY);
    for (Frame.Label l : frame.labels) {
      int y = height - (int)(l.y * height);
      g2d.drawLine(0, y, width, y);
    }
    g2d.setColor(Color.YELLOW);
    Path2D.Double path = new Path2D.Double();
    g2d.setClip(0, 0, width, height);
    boolean first = true;
    int first_x = 0, last_x = 0;
    for (Frame.Point p : frame.points) {
      int x = (int) (p.x * width);
      int y = height - (int) (p.y * height);
      if (first) {
        first_x = x;
        path.moveTo(x, y);
      } else {
        last_x = x;
        path.lineTo(x, y);
      }
      first = false;
    }
    path.lineTo(last_x, height + 5);
    path.lineTo(first_x, height + 5);
    path.closePath();
    path.setWindingRule(Path2D.WIND_NON_ZERO);
    g2d.draw(path);
    g2d.setPaint(new Color(255, 255, 0, 128));
    g2d.setStroke(new BasicStroke(0.0f));
    g2d.fill(path);
  }

  public void paint(Graphics g) {
    Graphics2D g2d = (Graphics2D)g;
    RenderFrame(current_frame_.get(), g2d);
  }
}

class Frame {
  public static class Label {
    double y;
    int value;
  }

  public static class Point {
    double x, y;
  }

  public Label labels[];
  public Point points[];

  // Reads the next frame from the InputStream. Returns null if no more frames
  // are available.
  static Frame ReadNextFrame(DataInputStream in) throws Exception {
    int num_labels;
    try {
      num_labels = Integer.reverseBytes(in.readInt());
    } catch (EOFException e) {
      return null;
    } catch (Exception e) {
      throw e;
    }
    int num_points = Integer.reverseBytes(in.readInt());
    Label labels[] = new Label[num_labels];
    Point points[] = new Point[num_points];
    for (int i = 0; i < labels.length; ++i) {
      labels[i] = new Label();
      labels[i].y = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
      labels[i].value = Integer.reverseBytes(in.readInt());
    }
    for (int i = 0; i < points.length; ++i) {
      points[i] = new Point();
      points[i].x = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
      points[i].y = Double.longBitsToDouble(Long.reverseBytes(in.readLong()));
    }
    return new Frame(labels, points);
  }

  protected Frame(Label labels_arg[], Point points_arg[]) {
    labels = labels_arg;
    points = points_arg;
  }
}
