import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.*;
import javax.swing.*;

public class Grapher extends JPanel {
  private Frame frames_[][];
  private AtomicInteger current_frame_ = new AtomicInteger(0);
  private long start_drawing_at_ = 0;
  private GrapherPanel graphers_[];

  public static void main(String args[]) throws Exception {
    JFrame frame = new JFrame("grapher");

    Grapher grapher = new Grapher(args[0]);
    System.err.printf("done creating grapher.\n");
    Container c = frame.getContentPane();
    c.setLayout(new FlowLayout());
    c.add(grapher);
    frame.setLocation(100, 100);
    frame.pack();
    frame.setVisible(true);
    grapher.Go();
  }

  public Grapher(String input_file) throws Exception {
    DataInputStream in = new DataInputStream(
        new BufferedInputStream(new FileInputStream(input_file)));

    int num_fields = Integer.reverseBytes(in.readInt());
    String captions[] = new String[num_fields];
    for (int i = 0; i < captions.length; ++i) {
      int size = Integer.reverseBytes(in.readInt());
      byte buf[] = new byte[size];
      in.readFully(buf);
      captions[i] = new String(buf);
    }
    int num_frames = Integer.reverseBytes(in.readInt());
    frames_ = new Frame[num_fields][num_frames];
    Frame frame;
    for (int i = 0; i < num_frames; ++i) {
      for (int j = 0; j < num_fields; ++j) {
        frames_[j][i] = Frame.ReadNextFrame(in);
      }
    }
    in.close();

    graphers_ = new GrapherPanel[num_fields];
    BoxLayout main_layout = new BoxLayout(this, BoxLayout.PAGE_AXIS);
    setLayout(main_layout);
    JPanel panels[] = new JPanel[num_fields];
    Color line_colors[] = {
        new Color(255, 255, 0), new Color(255, 0, 255), new Color(0, 255, 255),
        new Color(255, 128, 128),
    };
    Color fill_colors[] = {
        new Color(255, 255, 0, 128), new Color(255, 0, 255, 128),
        new Color(0, 255, 255, 128), new Color(255, 128, 128, 128),
    };
    for (int i = 0; i < panels.length; ++i) {
      panels[i] = new JPanel();
      graphers_[i] = new GrapherPanel(line_colors[i], fill_colors[i]);
      JLabel label = new JLabel(captions[i]);
      panels[i].setLayout(new BoxLayout(panels[i], BoxLayout.PAGE_AXIS));
      panels[i].add(label);
      panels[i].add(graphers_[i]);
    };
    JPanel line0 = new JPanel();
    JPanel line1 = new JPanel();
    BoxLayout line0_layout = new BoxLayout(line0, BoxLayout.LINE_AXIS);
    BoxLayout line1_layout = new BoxLayout(line1, BoxLayout.LINE_AXIS);
    line0.add(panels[0]);
    line0.add(panels[1]);
    line1.add(panels[2]);
    line1.add(panels[3]);
    add(line0);
    add(line1);
  }

  private void Go() {
    start_drawing_at_ = System.currentTimeMillis();
    new Thread(new UpdaterRunner()).start();
  }

  private void Update() {
    int fps = 30;
    int frame =
        (int)((System.currentTimeMillis() - start_drawing_at_) / 1000.0 * fps);
    if (current_frame_.get() != frame) {
      current_frame_.set(frame);
    }
    for (int i = 0; i < graphers_.length; ++i) {
      graphers_[i].setFrame(frames_[i][frame]);
      graphers_[i].repaint();
    }
  }

  private class UpdaterRunner implements Runnable {
    public void UpdaterRunner() {}
    public void run() {
      for (;;) {
        try {
          Thread.sleep(1);
          Update();
        } catch (Exception e) {
        }
      }
    }
  }
}

class GrapherPanel extends JPanel {
  private Frame frame_ = null;
  private Color line_color_ = null;
  private Color fill_color_ = null;
  private static final int kPadding = 5;
  public GrapherPanel(Color line_color, Color fill_color) {
    line_color_ = line_color;
    fill_color_ = fill_color;
  }
  public Dimension getMinimumSize() { return new Dimension(300, 300); }
  public Dimension getMaximumSize() { return getMinimumSize(); }
  public Dimension getPreferredSize() { return getMinimumSize(); }

  public void setFrame(Frame frame) {
    frame_ = frame;
  }
  public void paint(Graphics g) {
    Frame frame = frame_;
    if (frame == null) return;
    Graphics2D g2d = (Graphics2D)g;
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
    g2d.setColor(line_color_);
    Path2D.Double path = new Path2D.Double();
    g2d.setClip(0, 0, width, height);
    boolean first = true;
    int first_x = 0, last_x = 0;
    for (Frame.Point p : frame.points) {
      int x = (int)(p.x * width);
      int y = height - (int)(p.y * height);
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
    g2d.setPaint(fill_color_);
    g2d.setStroke(new BasicStroke(0.0f));
    g2d.fill(path);
  }
}

class Frame {
  public static class Label {
    double y;
    int value;
  }

  public static class Point { double x, y; }

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
