import java.awt.*;
import java.awt.event.*;
import java.awt.geom.*;
import java.io.*;
import java.util.*;
import java.util.concurrent.atomic.*;
import javax.swing.*;

public class Grapher extends JPanel {
  protected Frame frames_[];
  protected Image buffer_;
  protected TreeMap<Integer, Image> buffers_;
  private static final int kPadding = 5;
  private static final int kNumAdvancedFrames = 5;
  protected AtomicInteger current_frame_ = new AtomicInteger(0);
  protected int start_drawing_at_ = 0;
  protected AtomicInteger next_frame_ = new AtomicInteger(0);

  public static void main(String args[]) throws Exception {
    JFrame frame = new JFrame("grapher");

    Grapher grapher = new Grapher();

    grapher.setSize(500, 500);
    Container c = frame.getContentPane();
    c.setLayout(new FlowLayout());
    c.add(grapher);
    frame.pack();
    frame.setVisible(true);
  }

  public Grapher() throws Exception {
    DataInputStream in =
        new DataInputStream(
            new BufferedInputStream(
                new FileInputStream("time_series.out")));

    buffers_ = new TreeMap<int, image>();
    Vector<Frame> frames = new Vector<Frame>();
    Frame frame;
    while ((frame = Frame.ReadNextFrame(in)) != null) {
      frames.add(frame);
      System.err.printf("\r%d", frames.size());
    }
    frames_ = frames.toArray(new Frame[0]);
    System.err.printf("\n");
    in.close();
    System.err.printf("Read %d frames.\n", count);

    // Start the rendering threads, then wait for the first five frames to be
    // made before we start drawing.
    StartCleanupThread();
    for (int i = 0; i < kNumAdvancedFrames; ++i) {
      StartRenderingThread();
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

  private class RenderRunner implements Runnable {
    public RenderRunner() {}
    public void run() {
      for (;;) {
        int next_frame = GetNextFrameToRender();
        if (next_frame >= frames_.length) return;
        while (next_frame - current_frame_.get() > kNumAdvancedFrames) {
          Sleep(0); // Relinquish our time slice.
        }
        RenderFrame(next_frame);
      }
    }
  }

  private class CleanupRunner implements Runnable {
    public CleanupRunner() {}
    public void run() {
      while (KeepRendering()) {
        Vector<Integer> to_remove = new Vector<Integer>();
        synchronized (buffers_) {
          for (Map.Entry<Integer, Image> entry : buffers_) {
            if (entry.key() < current_frame_.get()) to_remove.add(entry.key());
          }
          for (int i : to_remove) {
            buffers_.remove(i);
          }
        }
        System.gc();
        Sleep(0);
      }
    }
  }

  private boolean KeepRendering() {
    synchronized (buffers_) {
      return !buffers_.containsKey(frames.length - 1);
    }
  }

  private int GetNextFrameToRender() {
    synchronized (buffers_) {
      return next_frame_.getAndIncrement();
    }
  }

  public void RenderFrame(int index) {
    Frame frame = frames_[index];

    int horizontal_offset = 20;
    int full_width = getWidth() - kPadding * 2;
    int width = full_width - horizontal_offset;
    int height = getHeight() - kPadding * 2;

    Image image = CreateOffscreenImage(width, height);
    Graphics2D g2d = (Graphics2D) image.getGraphics();

    g2d.setColor(Color.BLACK);
    g2d.fillRect(0, 0, width, height);
    g2d.setColor(Color.YELLOW);
    for (Frame.Point p : frame.points) {
      int point_width = 5;
      int point_height = 6;
      int x = (int) (p.x * width);
      int y = height - (int) (p.y * height);
      g2d.fill(new Ellipse2D.Float(x - point_width / 2,
                                   y - point_height / 2,
                                   point_width,
                                   point_height));
    }
    synchronized (buffers_) {
      buffers_.add(index, image);
    }
  }

  public void paint(Graphics g) {
    Graphics g2d = (Graphics2D)g;
    Image image = null;
    synchronized (buffers_) {
      image = buffers_.get(current_frame_.get());
    }
    if (image == null) return;
    g2d.drawImage(image, kPadding, kPadding, this);
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
